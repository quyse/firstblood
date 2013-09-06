#include "Painter.hpp"
#include "Geometry.hpp"
#include "GeometryFormats.hpp"

//*** Painter

const int Painter::shadowMapSize = 512;

Painter::DebugAttributes::DebugAttributes(ptr<Device> device, ptr<GeometryFormats> geometryFormats) :
	ab(device->CreateAttributeBinding(geometryFormats->debug.al)),
	position(geometryFormats->debug.alePosition),
	texcoord(geometryFormats->debug.aleTexcoord)
{}

Painter::ShadowSceneUniforms::ShadowSceneUniforms(ptr<Device> device) :
	group(NEW(UniformGroup(0))),
	viewProjTransform(group->AddUniform<mat4x4>())
{
	group->Finalize(device);
}

Painter::ShadowBlurUniforms::ShadowBlurUniforms(ptr<Device> device) :
	group(NEW(UniformGroup(0))),
	sourceSampler(0),
	blurDirection(group->AddUniform<vec2>())
{
	group->Finalize(device);
}

Painter::ColorSceneUniforms::ColorSceneUniforms(ptr<Device> device) :
	group(NEW(UniformGroup(0))),
	viewProjTransform(group->AddUniform<mat4x4>()),
	invViewProjTransform(group->AddUniform<mat4x4>()),
	cameraPosition(group->AddUniform<vec3>()),
	sunTransform(group->AddUniform<mat4x4>()),
	sunLight(group->AddUniform<vec3>()),
	sunShadowSampler(0)
{
	group->Finalize(device);
}

Painter::DebugModelUniforms::DebugModelUniforms(ptr<Device> device) :
	group(NEW(UniformGroup(1))),
	worldTransform(group->AddUniform<mat4x4>()),
	color(group->AddUniform<vec3>())
{
	group->Finalize(device);
}

Painter::ToneUniforms::ToneUniforms(ptr<Device> device) :
	group(NEW(UniformGroup(0))),
	luminanceKey(group->AddUniform<float>()),
	maxLuminance(group->AddUniform<float>()),
	sourceSampler(0)
{
	group->Finalize(device);
}

Painter::DebugModel::DebugModel(ptr<Geometry> geometry, const mat4x4& worldTransform, const vec3& color) :
	geometry(geometry),
	worldTransform(worldTransform),
	color(color)
{}

Painter::Painter(ptr<Device> device, ptr<Context> context, ptr<Presenter> presenter, ptr<Output> output, ptr<ShaderCache> shaderCache, ptr<GeometryFormats> geometryFormats) :
	device(device),
	context(context),
	presenter(presenter),
	output(output),
	screenWidth(0), screenHeight(0),
	shaderCache(shaderCache),
	geometryFormats(geometryFormats),

	debugAttributes(device, geometryFormats),
	shadowSceneUniforms(device),
	shadowBlurUniforms(device),
	colorSceneUniforms(device),
	debugModelUniforms(device),
	toneUniforms(device),

	iTexcoord(0),
	iDepth(1),
	fTarget(0)
{
	// создать ресурсы, зависящие от размера экрана
	ResizeScreen(output->GetWidth(), output->GetHeight());

	rbShadow = device->CreateRenderBuffer(shadowMapSize, shadowMapSize, PixelFormats::floatR16);
	dsbShadow = device->CreateDepthStencilBuffer(shadowMapSize, shadowMapSize, false);
	rbShadowBlur = device->CreateRenderBuffer(shadowMapSize, shadowMapSize, PixelFormats::floatR16);

	// геометрия полноэкранного прохода
	struct Quad
	{
		// вершина для фильтра
		struct Vertex
		{
			vec4 position;
			vec2 texcoord;
			vec2 gap;
		};

		ptr<VertexLayout> vl;
		ptr<AttributeLayout> al;
		ptr<AttributeLayoutSlot> als;
		Value<vec4> aPosition;
		Value<vec2> aTexcoord;

		ptr<VertexBuffer> vb;
		ptr<IndexBuffer> ib;

		ptr<AttributeBinding> ab;

		Quad(ptr<Device> device) :
			vl(NEW(VertexLayout(sizeof(Vertex)))),
			al(NEW(AttributeLayout())),
			als(al->AddSlot()),
			aPosition(al->AddElement(als, vl->AddElement(&Vertex::position))),
			aTexcoord(al->AddElement(als, vl->AddElement(&Vertex::texcoord)))
		{
			// разметка геометрии
			// геометрия полноэкранного квадрата
			Vertex vertices[] =
			{
				{ vec4(-1, -1, 0, 1), vec2(0, 1) },
				{ vec4(1, -1, 0, 1), vec2(1, 1) },
				{ vec4(1, 1, 0, 1), vec2(1, 0) },
				{ vec4(-1, 1, 0, 1), vec2(0, 0) }
			};
			unsigned short indices[] = { 0, 2, 1, 0, 3, 2 };

			vb = device->CreateStaticVertexBuffer(MemoryFile::CreateViaCopy(vertices, sizeof(vertices)), vl);
			ib = device->CreateStaticIndexBuffer(MemoryFile::CreateViaCopy(indices, sizeof(indices)), sizeof(unsigned short));

			ab = device->CreateAttributeBinding(al);
		}
	} quad(device);

	//** шейдеры

	Temp<vec4> tmpPosition;
	debugShaders.vs = shaderCache->GetVertexShader((
		tmpPosition = mul(shadowSceneUniforms.viewProjTransform, newvec4(debugAttributes.position, 1.0f)),
		setPosition(tmpPosition),
		iTexcoord = debugAttributes.texcoord,
		iDepth = tmpPosition["z"]
		));
	debugShaders.psShadow = shaderCache->GetPixelShader((
		fTarget = newvec4(iDepth, 0, 0, 0)
		));
	debugShaders.psColor = shaderCache->GetPixelShader((
		fTarget = newvec4(debugModelUniforms.color, 1.0f)
		));

	//** инициализировать состояния конвейера

	// shadow pass
	csShadow.viewportWidth = shadowMapSize;
	csShadow.viewportHeight = shadowMapSize;
	csShadow.renderBuffers[0] = rbShadow;
	csShadow.depthStencilBuffer = dsbShadow;
	shadowSceneUniforms.group->Apply(csShadow);

	// пиксельный шейдер для теней
	csShadow.pixelShader = shaderCache->GetPixelShader((
		fTarget = newvec4(iDepth, 0, 0, 0)
		));

	//** шейдеры и состояния постпроцессинга и размытия теней
	{
		ContextState csFilter;
		csFilter.attributeBinding = quad.ab;
		csFilter.vertexBuffers[0] = quad.vb;
		csFilter.indexBuffer = quad.ib;
		csFilter.depthTestFunc = ContextState::depthTestFuncAlways;
		csFilter.depthWrite = false;

		// промежуточные
		Interpolant<vec2> iTexcoord(0);
		// результат
		Fragment<vec4> fTarget(0);

		// вершинный шейдер - общий для всех постпроцессингов
		ptr<VertexShader> vertexShader = shaderCache->GetVertexShader((
			setPosition(quad.aPosition),
			iTexcoord = screenToTexture(quad.aPosition["xy"])
			));

		csFilter.vertexShader = vertexShader;

		// пиксельный шейдер для размытия тени
		ptr<PixelShader> psShadowBlur;
		{
			Temp<float> sum;
			Expression shader = (
				sum = 0
				);
			static const float taps[] = { 0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f };
			for(int i = 0; i < sizeof(taps) / sizeof(taps[0]); ++i)
				shader.Append((
					sum = sum + exp(shadowBlurUniforms.sourceSampler.Sample(iTexcoord + shadowBlurUniforms.blurDirection * Value<float>((float)i - 3))) * Value<float>(taps[i])
					));
			shader.Append((
				fTarget = newvec4(log(sum), 0, 0, 1)
				));
			psShadowBlur = shaderCache->GetPixelShader(shader);
		}

		// шейдер tone mapping
		ptr<PixelShader> psTone;
		{
			Temp<vec3> color;
			Temp<float> luminance, relativeLuminance, intensity;
			Expression shader = (
				iTexcoord,
				color = toneUniforms.sourceSampler.Sample(iTexcoord),
				luminance = dot(color, newvec3(0.2126f, 0.7152f, 0.0722f)),
				relativeLuminance = toneUniforms.luminanceKey * luminance,
				intensity = relativeLuminance * (Value<float>(1) + relativeLuminance / toneUniforms.maxLuminance) / (Value<float>(1) + relativeLuminance),
				color = saturate(color * (intensity / luminance)),
				// гамма-коррекция
				color = pow(color, newvec3(0.45f, 0.45f, 0.45f)),
				fTarget = newvec4(color, 1.0f)
			);
			psTone = shaderCache->GetPixelShader(shader);
		}

		csShadowBlur = csFilter;
		csTone = csFilter;

		// point sampler
		ptr<SamplerState> pointSampler = device->CreateSamplerState();
		pointSampler->SetFilter(SamplerState::filterPoint, SamplerState::filterPoint, SamplerState::filterPoint);
		pointSampler->SetWrap(SamplerState::wrapClamp, SamplerState::wrapClamp, SamplerState::wrapClamp);
		// linear sampler
		ptr<SamplerState> linearSampler = device->CreateSamplerState();
		linearSampler->SetFilter(SamplerState::filterLinear, SamplerState::filterLinear, SamplerState::filterLinear);
		linearSampler->SetWrap(SamplerState::wrapClamp, SamplerState::wrapClamp, SamplerState::wrapClamp);
		// point sampler with border=0
		ptr<SamplerState> pointBorderSampler = device->CreateSamplerState();
		pointBorderSampler->SetFilter(SamplerState::filterPoint, SamplerState::filterPoint, SamplerState::filterPoint);
		pointBorderSampler->SetWrap(SamplerState::wrapBorder, SamplerState::wrapBorder, SamplerState::wrapBorder);
		float borderColor[] = { 0, 0, 0, 0 };
		pointBorderSampler->SetBorderColor(borderColor);

		// состояние для размытия тени
		csShadowBlur.viewportWidth = shadowMapSize;
		csShadowBlur.viewportHeight = shadowMapSize;
		shadowBlurUniforms.sourceSampler.SetSamplerState(pointBorderSampler);
		shadowBlurUniforms.sourceSampler.Apply(csShadowBlur);
		shadowBlurUniforms.group->Apply(csShadowBlur);
		csShadowBlur.pixelShader = psShadowBlur;

		// tone mapping
		toneUniforms.sourceSampler.SetTexture(rbMain->GetTexture());
		toneUniforms.sourceSampler.SetSamplerState(pointSampler);
		toneUniforms.sourceSampler.Apply(csTone);
		toneUniforms.group->Apply(csTone);
		csTone.pixelShader = psTone;
	}
}

void Painter::ResizeScreen(int screenWidth, int screenHeight)
{
	if(screenWidth == this->screenWidth && screenHeight == this->screenHeight)
		return;

	this->screenWidth = screenWidth;
	this->screenHeight = screenHeight;

	rbMain = device->CreateRenderBuffer(screenWidth, screenHeight, PixelFormats::floatRGB32);
	dsbMain = device->CreateDepthStencilBuffer(screenWidth, screenHeight, false);
}

Value<vec3> Painter::ApplyQuaternion(Value<vec4> q, Value<vec3> v)
{
	return v + cross(q["xyz"], cross(q["xyz"], v) + v * q["w"]) * Value<float>(2);
}

void Painter::BeginFrame(float frameTime)
{
	this->frameTime = frameTime;

	debugModels.clear();
}

void Painter::SetCamera(const mat4x4& cameraViewProj, const vec3& cameraPosition)
{
	this->cameraViewProj = cameraViewProj;
	this->cameraInvViewProj = fromEigen(toEigen(cameraViewProj).inverse().eval());
	this->cameraPosition = cameraPosition;
}

void Painter::AddDebugModel(ptr<Geometry> geometry, const mat4x4& worldTransform, const vec3& color)
{
	debugModels.push_back(DebugModel(geometry, worldTransform, color));
}

void Painter::SetSceneLighting(const vec3& ambientLight, const vec3& sunLight, const vec3& sunDirection, const mat4x4& sunTransform)
{
	this->ambientLight = ambientLight;
	this->sunDirection = sunDirection;
	this->sunLight = sunLight;
}

void Painter::SetupPostprocess(float bloomLimit, float toneLuminanceKey, float toneMaxLuminance)
{
	this->bloomLimit = bloomLimit;
	this->toneLuminanceKey = toneLuminanceKey;
	this->toneMaxLuminance = toneMaxLuminance;
}

void Painter::Draw()
{
	float zeroColor[] = { 0, 0, 0, 0 };
	float farColor[] = { 1e8, 1e8, 1e8, 1e8 };

	// выполнить теневой проход
	{
		ContextState& cs = context->GetTargetState();

		cs.renderBuffers[0] = rbShadow;
		cs.depthStencilBuffer = dsbShadow;
		cs.viewportWidth = shadowMapSize;
		cs.viewportHeight = shadowMapSize;

		// указать трансформацию
		shadowSceneUniforms.viewProjTransform.SetValue(sunTransform);
		shadowSceneUniforms.group->Upload(context);

		// очистить карту теней
		context->ClearDepthStencilBuffer(dsbShadow, 1.0f);
		context->ClearRenderBuffer(rbShadow, farColor);

		//** рисуем отладочные модели

		cs.attributeBinding = debugAttributes.ab;
		cs.vertexShader = debugShaders.vs;
		cs.pixelShader = debugShaders.psShadow;
		// установить константный буфер
		debugModelUniforms.group->Apply(cs);

		// цикл по моделям
		for(size_t i = 0; i < debugModels.size(); ++i)
		{
			const DebugModel& model = debugModels[i];

			// установить геометрию
			cs.vertexBuffers[0] = model.geometry->GetVertexBuffer();
			cs.indexBuffer = model.geometry->GetIndexBuffer();
			// установить uniform'ы
			debugModelUniforms.worldTransform.SetValue(model.worldTransform);
			debugModelUniforms.color.SetValue(model.color);
			// и залить в GPU
			debugModelUniforms.group->Upload(context);

			// нарисовать
			context->Draw();
		}

		// выполнить размытие тени
		// первый проход
		cs = csShadowBlur;
		cs.renderBuffers[0] = rbShadowBlur;
		shadowBlurUniforms.sourceSampler.SetTexture(rbShadow->GetTexture());
		shadowBlurUniforms.sourceSampler.Apply(cs);
		shadowBlurUniforms.blurDirection.SetValue(vec2(1.0f / shadowMapSize, 0));
		shadowBlurUniforms.group->Upload(context);
		context->ClearRenderBuffer(rbShadowBlur, zeroColor);
		context->Draw();
		// второй проход
		cs = csShadowBlur;
		cs.renderBuffers[0] = rbShadow;
		shadowBlurUniforms.sourceSampler.SetTexture(rbShadowBlur->GetTexture());
		shadowBlurUniforms.sourceSampler.Apply(cs);
		shadowBlurUniforms.blurDirection.SetValue(vec2(0, 1.0f / shadowMapSize));
		shadowBlurUniforms.group->Upload(context);
		context->ClearRenderBuffer(rbShadow, zeroColor);
		context->Draw();
	}

	// очистить рендербуферы
	float color[4] = { 0, 0, 0, 1 };
	float colorDepth[4] = { 1, 1, 1, 1 };
	context->ClearRenderBuffer(rbMain, color);
	context->ClearDepthStencilBuffer(dsbMain, 1.0f);

	ContextState& cs = context->GetTargetState();

	cs.renderBuffers[0] = rbMain;
	cs.depthStencilBuffer = dsbMain;
	cs.viewportWidth = screenWidth;
	cs.viewportHeight = screenHeight;

	// установить параметры сцены
	colorSceneUniforms.viewProjTransform.SetValue(cameraViewProj);
	colorSceneUniforms.invViewProjTransform.SetValue(cameraInvViewProj);
	colorSceneUniforms.cameraPosition.SetValue(cameraPosition);
	colorSceneUniforms.sunTransform.SetValue(sunTransform);
	colorSceneUniforms.sunLight.SetValue(sunLight);
	colorSceneUniforms.sunShadowSampler.Apply(cs);
	colorSceneUniforms.group->Apply(cs);
	colorSceneUniforms.group->Upload(context);

	//** нарисовать отладочные модели

	cs.attributeBinding = debugAttributes.ab;
	cs.vertexShader = debugShaders.vs;
	cs.pixelShader = debugShaders.psColor;
	debugModelUniforms.group->Apply(cs);

	// нарисовать
	for(size_t i = 0; i < debugModels.size(); ++i)
	{
		const DebugModel& model = debugModels[i];

		// установить геометрию
		cs.vertexBuffers[0] = model.geometry->GetVertexBuffer();
		cs.indexBuffer = model.geometry->GetIndexBuffer();

		// установить uniform'ы
		debugModelUniforms.worldTransform.SetValue(model.worldTransform);
		debugModelUniforms.color.SetValue(model.color);
		debugModelUniforms.group->Upload(context);

		// нарисовать
		context->Draw();
	}

	// всё, теперь постпроцессинг
	float clearColor[] = { 0, 0, 0, 0 };

	// получить backbuffer
	ptr<RenderBuffer> rbBack = presenter->GetBackBuffer();

	// tone mapping
	cs = csTone;
	cs.renderBuffers[0] = rbBack;
	cs.viewportWidth = screenWidth;
	cs.viewportHeight = screenHeight;
	toneUniforms.luminanceKey.SetValue(toneLuminanceKey);
	toneUniforms.maxLuminance.SetValue(toneMaxLuminance);
	toneUniforms.group->Upload(context);
	context->ClearRenderBuffer(rbBack, zeroColor);
	context->Draw();
}
