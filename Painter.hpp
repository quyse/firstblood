#ifndef ___FIRSTBLOOD_PAINTER_HPP___
#define ___FIRSTBLOOD_PAINTER_HPP___

#include "general.hpp"
#include <unordered_map>

class Geometry;
class GeometryFormats;

/// Рисователь мира.
class Painter : public Object
{
private:
	/// Размер карты теней.
	static const int shadowMapSize;

	ptr<Device> device;
	ptr<Context> context;
	ptr<Presenter> presenter;
	ptr<Output> output;
	//** Размер экрана.
	/** Получается из Output на каждом кадре. */
	int screenWidth, screenHeight;
	/// Кэш бинарных шейдеров.
	ptr<ShaderCache> shaderCache;
	/// Форматы геометрии.
	ptr<GeometryFormats> geometryFormats;

	/// Основной буфер кадра.
	ptr<RenderBuffer> rbMain;
	/// Основная карта глубины.
	ptr<DepthStencilBuffer> dsbMain;
	/// Карта теней.
	ptr<RenderBuffer> rbShadow;
	/// Карта глубины для теней.
	ptr<DepthStencilBuffer> dsbShadow;
	/// Вспомогательный буфер для размытия тени.
	ptr<RenderBuffer> rbShadowBlur;

	//*** Атрибуты.
	/// Атрибуты отладочной геометрии.
	struct DebugAttributes
	{
		ptr<AttributeBinding> ab;
		Value<vec3> position;
		Value<vec2> texcoord;

		DebugAttributes(ptr<Device> device, ptr<GeometryFormats> geometryFormats);
	} debugAttributes;

	/// Uniform-группа сцены теней.
	struct ShadowSceneUniforms
	{
		ptr<UniformGroup> group;
		Uniform<mat4x4> viewProjTransform;

		ShadowSceneUniforms(ptr<Device> device);
	} shadowSceneUniforms;

	/// Uniform-группа размытия тени.
	struct ShadowBlurUniforms
	{
		ptr<UniformGroup> group;
		Sampler<float, 2> sourceSampler;
		Uniform<vec2> blurDirection;

		ShadowBlurUniforms(ptr<Device> device);
	} shadowBlurUniforms;

	/// Uniform-группа сцены цветового прохода.
	struct ColorSceneUniforms
	{
		ptr<UniformGroup> group;
		Uniform<mat4x4> viewProjTransform;
		Uniform<mat4x4> invViewProjTransform;
		Uniform<vec3> cameraPosition;
		Uniform<mat4x4> sunTransform;
		Uniform<vec3> sunLight;
		Sampler<float, 2> sunShadowSampler;

		ColorSceneUniforms(ptr<Device> device);
	} colorSceneUniforms;

	/// Uniform-группа отладочной модели.
	struct DebugModelUniforms
	{
		ptr<UniformGroup> group;
		/// Матрица мира.
		Uniform<mat4x4> worldTransform;
		/// Цвет модели.
		Uniform<vec3> color;

		DebugModelUniforms(ptr<Device> device);
	} debugModelUniforms;

	/// Uniform-группа tone mapping.
	struct ToneUniforms
	{
		ptr<UniformGroup> group;
		Uniform<float> luminanceKey;
		Uniform<float> maxLuminance;
		Sampler<vec3, 2> sourceSampler;

		ToneUniforms(ptr<Device> device);
	} toneUniforms;

	/// Интерполянты.
	Interpolant<vec2> iTexcoord;
	Interpolant<float> iDepth;

	/// Выходной цвет.
	Fragment<vec4> fTarget;

	//*** Разное для шейдеров.
	struct DebugShaders
	{
		ptr<VertexShader> vs;
		ptr<PixelShader> psShadow;
		ptr<PixelShader> psColor;
	} debugShaders;

	//** Состояния конвейера.
	/// Состояние для shadow pass.
	ContextState csShadow;
	/// Состояние для прохода размытия.
	ContextState csShadowBlur;
	/// Tone mapping.
	ContextState csTone;

private:
	void ResizeScreen(int screenWidth, int screenHeight);
	/// Повернуть вектор кватернионом.
	static Value<vec3> ApplyQuaternion(Value<vec4> q, Value<vec3> v);

	//*** Временные переменные пиксельного шейдера материала.
	Temp<vec4> tmpWorldPosition;
	Temp<vec2> tmpTexcoord;
	Temp<vec3> tmpNormal;
	Temp<vec3> tmpToCamera;
	Temp<vec4> tmpDiffuse, tmpSpecular;
	Temp<float> tmpSpecularExponent;
	Temp<vec3> tmpColor;

	/// Текущее время кадра.
	float frameTime;

	//*** зарегистрированные объекты для рисования

	// Текущая камера для opaque pass.
	mat4x4 cameraViewProj;
	mat4x4 cameraInvViewProj;
	vec3 cameraPosition;

	/// Настройки сцены.
	vec3 ambientLight;
	vec3 sunLight;
	vec3 sunDirection;
	mat4x4 sunTransform;

	float bloomLimit;
	float toneLuminanceKey;
	float toneMaxLuminance;

	/// Отладочная модель для рисования.
	struct DebugModel
	{
		ptr<Geometry> geometry;
		mat4x4 worldTransform;
		vec3 color;

		DebugModel(ptr<Geometry> geometry, const mat4x4& worldTransform, const vec3& color);
	};
	std::vector<DebugModel> debugModels;

public:
	Painter(ptr<Device> device, ptr<Context> context, ptr<Presenter> presenter, ptr<Output> output, ptr<ShaderCache> shaderCache, ptr<GeometryFormats> geometryFormats);

	/// Начать кадр.
	/** Очистить все регистрационные списки. */
	void BeginFrame(float frameTime);
	/// Установить камеру.
	void SetCamera(const mat4x4& cameraViewProj, const vec3& cameraPosition);
	/// Зарегистрировать отладочную модель.
	void AddDebugModel(ptr<Geometry> geometry, const mat4x4& worldTransform, const vec3& color);
	/// Установить параметры освещённости.
	void SetSceneLighting(const vec3& ambientLight, const vec3& sunLight, const vec3& sunDirection, const mat4x4& sunTransform);

	/// Установить параметры постпроцессинга.
	void SetupPostprocess(float bloomLimit, float toneLuminanceKey, float toneMaxLuminance);

	/// Выполнить рисование.
	void Draw();
};

#endif
