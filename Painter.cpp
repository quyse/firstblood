#include "Painter.hpp"
#include "Geometry.hpp"

//*** Painter

const int Painter::shadowMapSize = 512;
const int Painter::debugVerticesBufferCount = 1023;

Painter::DebugAttributes::DebugAttributes(ptr<Device> device, ptr<GeometryFormats> geometryFormats) :
	ab(device->CreateAttributeBinding(geometryFormats->debug.al)),
	position(geometryFormats->debug.alePosition),
	color(geometryFormats->debug.aleColor)
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

Painter::SkyUniforms::SkyUniforms(ptr<Device> device) :
	group(NEW(UniformGroup(0))),
	invViewProjTransform(group->AddUniform<mat4x4>()),
	cameraPosition(group->AddUniform<vec3>())
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

Painter::ToneUniforms::ToneUniforms(ptr<Device> device) :
	group(NEW(UniformGroup(0))),
	luminanceKey(group->AddUniform<float>()),
	maxLuminance(group->AddUniform<float>()),
	sourceSampler(0)
{
	group->Finalize(device);
}

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
	skyUniforms(device),
	colorSceneUniforms(device),
	toneUniforms(device),

	iTexcoord(0),
	iColor(1),
	iDepth(2),
	fTarget(0)
{
	// создать ресурсы, зависящие от размера экрана
	ResizeScreen(output->GetWidth(), output->GetHeight());

	rbShadow = device->CreateRenderBuffer(shadowMapSize, shadowMapSize, PixelFormats::floatR16);
	dsbShadow = device->CreateDepthStencilBuffer(shadowMapSize, shadowMapSize, false);
	fbShadow = device->CreateFrameBuffer();
	fbShadow->SetColorBuffer(0, rbShadow);
	fbShadow->SetDepthStencilBuffer(dsbShadow);
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
	vsDebug = shaderCache->GetVertexShader((
		tmpPosition = mul(colorSceneUniforms.viewProjTransform, newvec4(debugAttributes.position, 1.0f)),
		//tmpPosition = newvec4(debugAttributes.position, 1.0f),
		setPosition(tmpPosition),
		iColor = debugAttributes.color
		));
	psDebug = shaderCache->GetPixelShader((
		fTarget = newvec4(iColor, 1)
		));

	//** шейдеры постпроцессинга и размытия теней
	{
		abFilter = quad.ab;
		vbFilter = quad.vb;
		ibFilter = quad.ib;

		// промежуточные
		Interpolant<vec2> iTexcoord(0);
		// результат
		Fragment<vec4> fTarget(0);

		// вершинный шейдер - общий для всех постпроцессингов
		vsFilter = shaderCache->GetVertexShader((
			setPosition(quad.aPosition),
			iTexcoord = screenToTexture(quad.aPosition["xy"])
			));

		// шейдеры неба
		vsSky = shaderCache->GetVertexShader((
			setPosition(newvec4(quad.aPosition["xy"], 1.0f, 1.0f)),
			iTexcoord = screenToTexture(quad.aPosition["xy"])
			));
		Temp<vec4> p;
		Temp<float> q;
		psSky = shaderCache->GetPixelShader((
			p = mul(skyUniforms.invViewProjTransform, newvec4(iTexcoord * newvec2(2.0f, -2.0f) + newvec2(-1.0f, 1.0f), 1.0f, 1.0f)),
			q = normalize(p["xyz"] / p["w"] - skyUniforms.cameraPosition)["z"] * Value<float>(0.5f) + Value<float>(0.5f),
			fTarget = newvec4(q, q, q, 1)
			));

		// шейдер tone mapping
		{
			Temp<vec3> color;
			Temp<float> luminance, relativeLuminance, intensity;
			Expression shader = (
				iTexcoord,
				color = toneUniforms.sourceSampler.Sample(iTexcoord),
				fTarget = newvec4(color, 1.0f)
#if 0
				luminance = dot(color, newvec3(0.2126f, 0.7152f, 0.0722f)),
				relativeLuminance = toneUniforms.luminanceKey * luminance,
				intensity = relativeLuminance * (Value<float>(1) + relativeLuminance / toneUniforms.maxLuminance) / (Value<float>(1) + relativeLuminance),
				color = saturate(color * (intensity / luminance)),
				// гамма-коррекция
				color = pow(color, newvec3(0.45f, 0.45f, 0.45f)),
				fTarget = newvec4(color, 1.0f)
#endif
			);
			psTone = shaderCache->GetPixelShader(shader);
		}

		// point sampler
		ssPoint = device->CreateSamplerState();
		ssPoint->SetFilter(SamplerState::filterPoint, SamplerState::filterPoint, SamplerState::filterPoint);
		ssPoint->SetWrap(SamplerState::wrapClamp, SamplerState::wrapClamp, SamplerState::wrapClamp);
		// linear sampler
		ssLinear = device->CreateSamplerState();
		ssLinear->SetFilter(SamplerState::filterLinear, SamplerState::filterLinear, SamplerState::filterLinear);
		ssLinear->SetWrap(SamplerState::wrapClamp, SamplerState::wrapClamp, SamplerState::wrapClamp);
		// point sampler with border=0
		ssPointBorder = device->CreateSamplerState();
		ssPointBorder->SetFilter(SamplerState::filterPoint, SamplerState::filterPoint, SamplerState::filterPoint);
		ssPointBorder->SetWrap(SamplerState::wrapBorder, SamplerState::wrapBorder, SamplerState::wrapBorder);
		float borderColor[] = { 0, 0, 0, 0 };
		ssPointBorder->SetBorderColor(borderColor);
	}

	vbDebug = device->CreateDynamicVertexBuffer(debugVerticesBufferCount * sizeof(GeometryFormats::Debug::Vertex), geometryFormats->debug.vl);
	{
		ptr<File> file = NEW(MemoryFile(debugVerticesBufferCount * sizeof(unsigned short)));
		unsigned short* indices = (unsigned short*)file->GetData();
		for(int i = 0; i < debugVerticesBufferCount; ++i)
			indices[i] = i;
		ibDebug = device->CreateStaticIndexBuffer(file, sizeof(unsigned short));
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
	fbMain = device->CreateFrameBuffer();
	fbMain->SetColorBuffer(0, rbMain);
	fbMain->SetDepthStencilBuffer(dsbMain);
	fbPreMain = device->CreateFrameBuffer();
	fbPreMain->SetColorBuffer(0, rbMain);
}

Value<vec3> Painter::ApplyQuaternion(Value<vec4> q, Value<vec3> v)
{
	return v + cross(q["xyz"], cross(q["xyz"], v) + v * q["w"]) * Value<float>(2);
}

void Painter::BeginFrame(float frameTime)
{
	this->frameTime = frameTime;

	debugVertices.clear();
}

void Painter::SetCamera(const mat4x4& cameraViewProj, const vec3& cameraPosition)
{
	this->cameraViewProj = cameraViewProj;
	this->cameraInvViewProj = fromEigen(toEigen(cameraViewProj).inverse().eval());
	this->cameraPosition = cameraPosition;
}

void Painter::SetSceneLighting(const vec3& ambientLight, const vec3& sunLight, const vec3& sunDirection, const mat4x4& sunTransform)
{
	this->ambientLight = ambientLight;
	this->sunDirection = sunDirection;
	this->sunLight = sunLight;
}

void Painter::DebugDrawLine(const vec3& a, const vec3& b, const vec3& color, float thickness, const vec3& normal)
{
	vec3 line = b - a;
	float len = length(line);
	if(len < 1e-8)
		return;
	line /= len;
	vec3 side = cross(line, normal) * thickness;
	debugVertices.push_back(GeometryFormats::Debug::Vertex(a + side, color));
	debugVertices.push_back(GeometryFormats::Debug::Vertex(a - side, color));
	debugVertices.push_back(GeometryFormats::Debug::Vertex(b - side, color));
	debugVertices.push_back(GeometryFormats::Debug::Vertex(a + side, color));
	debugVertices.push_back(GeometryFormats::Debug::Vertex(b - side, color));
	debugVertices.push_back(GeometryFormats::Debug::Vertex(b + side, color));
}

void Painter::DebugDrawRectangle(float x1, float y1, float x2, float y2, float z, const vec3& color, float thickness, const vec3& normal)
{
	DebugDrawLine(vec3(x1, y1, z), vec3(x2, y1, z), color, thickness, normal);
	DebugDrawLine(vec3(x2, y1, z), vec3(x2, y2, z), color, thickness, normal);
	DebugDrawLine(vec3(x2, y2, z), vec3(x1, y2, z), color, thickness, normal);
	DebugDrawLine(vec3(x1, y2, z), vec3(x1, y1, z), color, thickness, normal);
}

void Painter::DebugDrawAABB(const vec3& a, const vec3& b, const vec3& color)
{
	DebugDrawCube(CreateTranslationMatrix((a + b) * 0.5f) * CreateScalingMatrix((b - a) * 0.5f), color);
}

void Painter::DebugDrawCube(const mat4x4& transform, const vec3& color)
{
	GeometryFormats::Debug::Vertex v[8];
	int k = 0;
	for(int z = -1; z <= 1; z += 2)
		for(int y = -1; y <= 1; y += 2)
			for(int x = -1; x <= 1; x += 2)
			{
				vec4 r = transform * vec3((float)x, (float)y, (float)z);
				v[k++] = GeometryFormats::Debug::Vertex(vec3(r.x, r.y, r.z), color);
			}
	static const int n[][4] =
	{
		{ 0, 1, 3, 2 },
		{ 0, 4, 5, 1 },
		{ 0, 2, 6, 4 },
		{ 3, 7, 6, 2 },
		{ 3, 1, 5, 7 },
		{ 4, 6, 7, 5 }
	};
	static const int q[] = { 0, 1, 2, 0, 2, 3 };
	for(int i = 0; i < 6; ++i)
		for(int j = 0; j < 6; ++j)
			debugVertices.push_back(v[n[i][q[j]]]);
}

void Painter::DebugDrawCircle(const vec3& center, float radius, const vec3& color, uint segmentsCount)
{
	for (size_t i = 0; i < segmentsCount; ++i)
	{
		float startAngle = i * 2 * (float)M_PI / segmentsCount;
		float endAngle = ((i + 1) % segmentsCount) * 2 * (float)M_PI / segmentsCount;
		vec3 start = center + vec3(cos(startAngle) * radius, sin(startAngle) * radius, center.z);
		vec3 end = center + vec3(cos(endAngle) * radius, sin(endAngle) * radius, center.z);
		DebugDrawLine(start, end, color);
	}
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

	Context::LetViewport lv(context, screenWidth, screenHeight);
	Context::LetCullMode lcm(context, Context::cullModeBack);

	// предварительное рисование и небо
	{
		Context::LetFrameBuffer lfb(context, fbPreMain);

		// очистить рендербуферы
		float color[4] = { 0.1f, 0, 0, 1 };
		context->ClearColor(0, color);

		// нарисовать небо
		skyUniforms.invViewProjTransform.SetValue(cameraInvViewProj);
		skyUniforms.cameraPosition.SetValue(cameraPosition);
		skyUniforms.group->Upload(context);
		Context::LetUniformBuffer lubSky(context, skyUniforms.group);
		Context::LetVertexShader lvs(context, vsSky);
		Context::LetPixelShader lps(context, psSky);
		Context::LetVertexBuffer lvb(context, 0, vbFilter);
		Context::LetIndexBuffer lib(context, ibFilter);
		Context::LetAttributeBinding lab(context, abFilter);
		Context::LetDepthTestFunc ldtf(context, Context::depthTestFuncAlways);
		Context::LetDepthWrite ldw(context, false);
		Context::LetCullMode lcm(context, Context::cullModeNone);
		context->Draw();
	}

	// основное рисование
	{
		Context::LetFrameBuffer lfb(context, fbMain);

		context->ClearDepth(1.0f);

		colorSceneUniforms.viewProjTransform.SetValue(cameraViewProj);
		colorSceneUniforms.invViewProjTransform.SetValue(cameraInvViewProj);
		colorSceneUniforms.cameraPosition.SetValue(cameraPosition);
		colorSceneUniforms.sunTransform.SetValue(sunTransform);
		colorSceneUniforms.sunLight.SetValue(sunLight);
		colorSceneUniforms.group->Upload(context);
		Context::LetUniformBuffer lubColorScene(context, colorSceneUniforms.group);

		//** нарисовать отладочную геометрию

		Context::LetAttributeBinding lab(context, debugAttributes.ab);
		Context::LetVertexShader lvs(context, vsDebug);
		Context::LetPixelShader lps(context, psDebug);
		Context::LetVertexBuffer lvb(context, 0, vbDebug);
		Context::LetIndexBuffer lib(context, ibDebug);
		Context::LetCullMode lcm(context, Context::cullModeNone);

		for(int i = 0; i < (int)debugVertices.size(); i += debugVerticesBufferCount)
		{
			int verticesCount = std::min((int)debugVertices.size() - i, debugVerticesBufferCount);
			context->UploadVertexBufferData(vbDebug, &debugVertices[i], verticesCount * sizeof(GeometryFormats::Debug::Vertex));

			context->Draw(verticesCount);
		}
	}

	// всё, теперь постпроцессинг
	{
		float clearColor[] = { 0, 0, 0, 0 };

		// tone mapping
		Context::LetFrameBuffer lfb(context, presenter->GetFrameBuffer());
		Context::LetViewport lv(context, screenWidth, screenHeight);

		toneUniforms.luminanceKey.SetValue(toneLuminanceKey);
		toneUniforms.maxLuminance.SetValue(toneMaxLuminance);
		toneUniforms.group->Upload(context);
		Context::LetUniformBuffer lubTone(context, toneUniforms.group);
		Context::LetSampler lsSource(context, toneUniforms.sourceSampler, rbMain->GetTexture(), ssPoint);
		Context::LetVertexBuffer lvb(context, 0, vbFilter);
		Context::LetIndexBuffer lib(context, ibFilter);
		Context::LetAttributeBinding lab(context, abFilter);
		Context::LetVertexShader lvs(context, vsFilter);
		Context::LetPixelShader lps(context, psTone);
		Context::LetDepthTestFunc ldtf(context, Context::depthTestFuncAlways);
		Context::LetDepthWrite ldw(context, false);

		context->ClearColor(0, clearColor);

		context->Draw();
	}
}
