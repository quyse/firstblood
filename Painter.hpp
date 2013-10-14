#ifndef ___FIRSTBLOOD_PAINTER_HPP___
#define ___FIRSTBLOOD_PAINTER_HPP___

#include "general.hpp"
#include "GeometryFormats.hpp"
#include <unordered_map>

class Geometry;

/// Рисователь мира.
class Painter : public Object
{
private:
	/// Размер карты теней.
	static const int shadowMapSize;
	/// Количество индексов в буфере отладочной геометрии.
	static const int debugVerticesBufferCount;

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

	//*** Фреймбуферы.
	ptr<FrameBuffer> fbMain, fbPreMain, fbShadow;
	ptr<FrameBuffer> fbShadowBlur1, fbShadowBlur2;

	//*** Геометрия.
	ptr<AttributeBinding> abFilter;
	ptr<VertexBuffer> vbFilter;
	ptr<IndexBuffer> ibFilter;
	ptr<VertexBuffer> vbDebug;
	ptr<IndexBuffer> ibDebug;

	//*** Шейдеры.
	ptr<VertexShader> vsFilter;
	ptr<VertexShader> vsSky;
	ptr<PixelShader> psSky;
	ptr<VertexShader> vsDebug;
	ptr<PixelShader> psDebug;
	ptr<PixelShader> psTone;

	//*** Семплеры.
	ptr<SamplerState> ssPoint, ssLinear, ssPointBorder;

	//*** Атрибуты.
	/// Атрибуты отладочной геометрии.
	struct DebugAttributes
	{
		ptr<AttributeBinding> ab;
		Value<vec3> position;
		Value<vec3> color;

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

	/// Uniform-группа неба.
	struct SkyUniforms
	{
		ptr<UniformGroup> group;
		Uniform<mat4x4> invViewProjTransform;
		Uniform<vec3> cameraPosition;

		SkyUniforms(ptr<Device> device);
	} skyUniforms;

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
	Interpolant<vec3> iColor;
	Interpolant<float> iDepth;

	/// Выходной цвет.
	Fragment<vec4> fTarget;

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

	/// Вершины отладочной геометрии.
	std::vector<GeometryFormats::Debug::Vertex> debugVertices;

public:
	Painter(ptr<Device> device, ptr<Context> context, ptr<Presenter> presenter, ptr<Output> output, ptr<ShaderCache> shaderCache, ptr<GeometryFormats> geometryFormats);

	/// Начать кадр.
	/** Очистить все регистрационные списки. */
	void BeginFrame(float frameTime);
	/// Установить камеру.
	void SetCamera(const mat4x4& cameraViewProj, const vec3& cameraPosition);
	/// Установить параметры освещённости.
	void SetSceneLighting(const vec3& ambientLight, const vec3& sunLight, const vec3& sunDirection, const mat4x4& sunTransform);

	//*** Отладочное рисование.
	void DebugDrawLine(const vec3& a, const vec3& b, const vec3& color, float thickness = 0.1f, const vec3& normal = vec3(0, 0, 1));
	void DebugDrawRectangle(float x1, float y1, float x2, float y2, float z, const vec3& color, float thickness = 0.1f, const vec3& normal = vec3(0, 0, 1));
	void DebugDrawAABB(const vec3& a, const vec3& b, const vec3& color);
	/// Нарисовать куб [-1, 1] с заданной трансформацией.
	void DebugDrawCube(const mat4x4& transform, const vec3& color);
	//void DebugDrawSphere(const vec3& center, float radius, const vec3& color, int alphaCount = 8, int betaCount = 8);
	void DebugDrawCircle(const vec3& center, float radius, const vec3& color, uint segmentsCount);

	/// Установить параметры постпроцессинга.
	void SetupPostprocess(float bloomLimit, float toneLuminanceKey, float toneMaxLuminance);

	/// Выполнить рисование.
	void Draw();
};

#endif
