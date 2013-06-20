#ifndef __MC_GRAPHICS__
#define __MC_GRAPHICS__

#include "core.h"

////////////////////////////////////////////////////////////////////////////////

typedef struct __MCGContext *MCGContextRef;
typedef struct __MCGPath *MCGPathRef;
typedef struct __MCGImage *MCGImageRef;

typedef struct __MCGDashes *MCGDashesRef;

////////////////////////////////////////////////////////////////////////////////

typedef float MCGFloat;
typedef uint32_t MCGColor;

struct MCGPoint
{
	MCGFloat x, y;
};

struct MCGSize
{
	MCGFloat width, height;
};

struct MCGRectangle
{
	MCGPoint origin;
	MCGSize size;
};

struct MCGAffineTransform
{
	MCGFloat a, b, c, d;
	MCGFloat tx, ty;
};

////////////////////////////////////////////////////////////////////////////////

enum MCGFillRule
{
	kMCGFillRuleNonZero,
	kMCGFillRuleEvenOdd
};

enum MCGBlendMode
{
	kMCGBlendModeClear,
	kMCGBlendModeCopy,
	kMCGBlendModeSourceOver,
	kMCGBlendModeSourceIn,
	kMCGBlendModeSourceOut,
	kMCGBlendModeSourceAtop,
	kMCGBlendModeDestinationOver,
	kMCGBlendModeDestinationIn,
	kMCGBlendModeDestinationOut,
	kMCGBlendModeDestinationAtop,
	kMCGBlendModeXor,
	kMCGBlendModePlusDarker,
	kMCGBlendModePlusLighter,
	kMCGBlendModeMultiply,
	kMCGBlendModeScreen,
	kMCGBlendModeOverlay,
	kMCGBlendModeDarken,
	kMCGBlendModeLighten,
	kMCGBlendModeColorDodge,
	kMCGBlendModeColorBurn,
	kMCGBlendModeSoftLight,
	kMCGBlendModeHardLight,
	kMCGBlendModeDifference,
	kMCGBlendModeExclusion,
	kMCGBlendModeHue,
	kMCGBlendModeSaturation,
	kMCGBlendModeColor,
	kMCGBlendModeLuminosity,
	
	kMCGBlendModeCount,
};

enum MCGJoinStyle
{
	kMCGJoinStyleBevel,
	kMCGJoinStyleRound,
	kMCGJoinStyleMiter,
};

enum MCGCapStyle
{
	kMCGCapStyleButt,
	kMCGCapStyleRound,
	kMCGCapStyleSquare
};

enum MCGRasterFormat
{
	kMCGRasterFormat_xRGB, // no alpha channel
	kMCGRasterFormat_ARGB, // premultiplied alpha channel
	kMCGRasterFormat_U_ARGB, // Unpremultipled alpha channel
	kMCGRasterFormat_A, // alpha mask
};

enum MCGImageFilter
{
	kMCGImageFilterNearest,
	kMCGImageFilterBilinear,
	kMCGImageFilterBicubic,
};

enum MCGGradientFunction
{
	kMCGGradientFunctionLinear,
	kMCGGradientFunctionRadial,
	kMCGGradientFunctionConical,
	kMCGGradientFunctionSweep,
};

enum MCGGradientTileMode
{
	kMCGGradientTileModeClamp,
	kMCGGradientTileModeRepeat,
	kMCGGradientTileModeMirror,	
};

struct MCGRaster
{
	MCGRasterFormat format;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	void *pixels;
};

struct MCGStrokeAttr
{
	MCGFloat width;
	MCGJoinStyle join_style;
	MCGCapStyle cap_style;
	MCGFloat miter_limit;
	MCGDashesRef dashes;
};

////////////////////////////////////////////////////////////////////////////////

inline MCGRectangle MCGRectangleMake(MCGFloat p_x, MCGFloat p_y, MCGFloat p_width, MCGFloat p_height)
{
	MCGRectangle t_rect;
	t_rect . origin . x = p_x;
	t_rect . origin . y = p_y;
	t_rect . size . width = p_width;
	t_rect . size . height = p_height;
	return t_rect;
}

inline MCGRectangle MCGRectangleTranslate(MCGRectangle p_rect, MCGFloat p_dx, MCGFloat p_dy)
{
	MCGRectangle t_rect = p_rect;
	t_rect.origin.x += p_dx;
	t_rect.origin.y += p_dy;
	
	return t_rect;
}

MCGRectangle MCGRectangleIntersection(MCGRectangle rect_1, MCGRectangle rect_2);

inline MCGPoint MCGPointMake(MCGFloat p_x, MCGFloat p_y)
{
	MCGPoint t_point;
	t_point . x = p_x;
	t_point . y = p_y;
	return t_point;
}

////////////////////////////////////////////////////////////////////////////////

bool MCGImageCreateWithRaster(const MCGRaster& raster, MCGImageRef& r_image);
bool MCGImageCreateWithRasterAndRelease(const MCGRaster &raster, MCGImageRef &r_image);
bool MCGImageCreateWithData(const void *bytes, uindex_t byte_count, MCGImageRef& r_image);
bool MCGImageCreateWithFilename(const char *filename, MCGImageRef& r_image);

bool MCGImageGetRaster(MCGImageRef p_image, MCGRaster &r_raster);

MCGImageRef MCGImageRetain(MCGImageRef image);
void MCGImageRelease(MCGImageRef image);

bool MCGImageIsValid(MCGImageRef image);

int32_t MCGImageGetWidth(MCGImageRef image);
int32_t MCGImageGetHeight(MCGImageRef image);

MCGSize MCImageGetSize(MCGImageRef image);

////////////////////////////////////////////////////////////////////////////////

typedef uint8_t MCGPathCommand;
enum
{
	kMCGPathCommandEnd,
	kMCGPathCommandMoveTo,
	kMCGPathCommandLineTo,
	kMCGPathCommandCurveTo,
	kMCGPathCommandQuadCurveTo,
	kMCGPathCommandCloseSubpath,
};

bool MCGPathCreate(const MCGPathCommand *commands, const MCGFloat *parameters, MCGPathRef& r_path);
bool MCGPathCreateMutable(MCGPathRef& r_path);

MCGPathRef MCGPathRetain(MCGPathRef path);
void MCGPathRelease(MCGPathRef path);

bool MCGPathIsValid(MCGPathRef path);

void MCGPathCopy(MCGPathRef path, MCGPathRef& r_new_path);
void MCGPathCopyAndRelease(MCGPathRef path, MCGPathRef& r_new_path);
void MCGPathMutableCopy(MCGPathRef path, MCGPathRef& r_new_path);
void MCGPathMutableCopyAndRelease(MCGPathRef path, MCGPathRef& r_new_path);

void MCGPathAddRectangle(MCGPathRef path, MCGRectangle bounds);
void MCGPathAddRoundedRectangle(MCGPathRef path, MCGRectangle bounds, MCGSize corner_radii);
void MCGPathAddEllipse(MCGPathRef path, MCGPoint center, MCGSize radii, MCGFloat rotation);
void MCGPathAddArc(MCGPathRef path, MCGPoint center, MCGSize radii, MCGFloat rotation, MCGFloat start_angle, MCGFloat finish_angle);
void MCGPathAddSector(MCGPathRef path, MCGPoint center, MCGSize radii, MCGFloat rotation, MCGFloat start_angle, MCGFloat finish_angle);
void MCGPathAddSegment(MCGPathRef path, MCGPoint center, MCGSize radii, MCGFloat rotation, MCGFloat start_angle, MCGFloat finish_angle);
void MCGPathAddLine(MCGPathRef path, MCGPoint start, MCGPoint finish);
void MCGPathAddPolygon(MCGPathRef path, const MCGPoint *points, uindex_t arity);
void MCGPathAddPolyline(MCGPathRef path, const MCGPoint *points, uindex_t arity);
void MCGPathAddPath(MCGPathRef self, MCGPathRef path);

void MCGPathMoveTo(MCGPathRef path, MCGPoint end_point);
void MCGPathLineTo(MCGPathRef path, MCGPoint end_point);
void MCGPathQuadraticTo(MCGPathRef path, MCGPoint control_point, MCGPoint end_point);
void MCGPathCubicTo(MCGPathRef path, MCGPoint first_control_point, MCGPoint second_control_point, MCGPoint end_point);
void MCGPathArcTo(MCGPathRef path, MCGSize radii, MCGFloat rotation, bool large_arc, bool sweep, MCGPoint end_point);
void MCGPathCloseSubpath(MCGPathRef path);

void MCGPathThicken(MCGPathRef path, const MCGStrokeAttr& attr, MCGPathRef& r_thick_path);
void MCGPathFlatten(MCGPathRef path, MCGFloat flatness, MCGPathRef& r_flat_path);
void MCGPathSimplify(MCGPathRef path, MCGPathRef& r_simple_path);

////////////////////////////////////////////////////////////////////////////////

bool MCGContextCreate(uint32_t p_width, uint32_t p_height, bool p_alpha, MCGContextRef& r_context);
bool MCGContextCreateWithPixels(uint32_t p_width, uint32_t p_height, uint32_t p_stride, void *p_pixels, bool p_alpha, MCGContextRef& r_context);
bool MCGContextCreateWithRaster(MCGRaster &p_raster, MCGContextRef &r_context);

MCGContextRef MCGContextRetain(MCGContextRef context);
void MCGContextRelease(MCGContextRef context);

// Returns whether the current context context is valid. If an error
// occurs when calling any method on a context context, it will become
// invalid and all further operations will be no-ops.
bool MCGContextIsValid(MCGContextRef context);

// Graphics state operations
void MCGContextSave(MCGContextRef context);
void MCGContextRestore(MCGContextRef context);

// General attributes
void MCGContextSetFlatness(MCGContextRef context, MCGFloat flatness);
void MCGContextSetShouldAntialias(MCGContextRef context, bool should_antialias);

// Layer attributes and manipulation - bitmap effect options would be added here also.
void MCGContextSetOpacity(MCGContextRef context, MCGFloat opacity);
void MCGContextSetBlendMode(MCGContextRef context, MCGBlendMode mode);

void MCGContextBegin(MCGContextRef context);
void MCGContextEnd(MCGContextRef context);

void MCGContextClipToRect(MCGContextRef context, MCGRectangle rect);
void MCGContextResetClip(MCGContextRef context);

// Fill attributes
void MCGContextSetFillRule(MCGContextRef context, MCGFillRule rule);
void MCGContextSetFillOpacity(MCGContextRef context, MCGFloat opacity);
void MCGContextSetFillRGBAColor(MCGContextRef context, MCGFloat red, MCGFloat green, MCGFloat blue, MCGFloat alpha);
void MCGContextSetFillPattern(MCGContextRef context, MCGImageRef image, MCGAffineTransform transform, MCGImageFilter filter);
void MCGContextSetFillGradient(MCGContextRef context, MCGGradientFunction function, const MCGFloat* stops, const MCGColor* colors, uindex_t ramp_length, bool mirror, bool wrap, uint32_t repeats, MCGAffineTransform transform, MCGImageFilter filter);

// Stroke attributes
void MCGContextSetStrokeOpacity(MCGContextRef context, MCGFloat opacity);
void MCGContextSetStrokeRGBAColor(MCGContextRef context, MCGFloat red, MCGFloat green, MCGFloat blue, MCGFloat alpha);
void MCGContextSetStrokePattern(MCGContextRef context, MCGImageRef image, MCGAffineTransform transform, MCGImageFilter filter);
void MCGContextSetStrokeGradient(MCGContextRef context, MCGGradientFunction function, const MCGFloat* stops, const MCGColor* colors, uindex_t ramp_length, bool mirror, bool wrap, uint32_t repeats, MCGAffineTransform transform, MCGImageFilter filter);
void MCGContextSetStrokeWidth(MCGContextRef context, MCGFloat width);
void MCGContextSetStrokeMiterLimit(MCGContextRef context, MCGFloat limit);
void MCGContextSetStrokeJoinStyle(MCGContextRef context, MCGJoinStyle style);
void MCGContextSetStrokeCapStyle(MCGContextRef context, MCGCapStyle style);
void MCGContextSetStrokeDashes(MCGContextRef context, MCGFloat phase, const MCGFloat *lengths, uindex_t arity);

// Transforms - concatenated with the current CTM.
void MCGContextConcatCTM(MCGContextRef context, MCGAffineTransform transform);
void MCGContextRotateCTM(MCGContextRef context, MCGFloat angle);
void MCGContextTranslateCTM(MCGContextRef context, MCGFloat xoffset, MCGFloat yoffset);
void MCGContextScaleCTM(MCGContextRef context, MCGFloat xscale, MCGFloat yscale);
void MCGContextResetCTM(MCGContextRef context);

// Shape primitives - these add to the current path.
void MCGContextAddRectangle(MCGContextRef context, MCGRectangle bounds);
void MCGContextAddRoundedRectangle(MCGContextRef context, MCGRectangle bounds, MCGSize corner_radii);
void MCGContextAddEllipse(MCGContextRef context, MCGPoint center, MCGSize radii, MCGFloat rotation);
void MCGContextAddArc(MCGContextRef context, MCGPoint center, MCGSize radii, MCGFloat p_rotation, MCGFloat start_angle, MCGFloat finish_angle);
void MCGContextAddSector(MCGContextRef context, MCGPoint center, MCGSize radii, MCGFloat rotation, MCGFloat start_angle, MCGFloat finish_angle);
void MCGContextAddSegment(MCGContextRef context, MCGPoint center, MCGSize radii, MCGFloat rotation, MCGFloat start_angle, MCGFloat finish_angle);
void MCGContextAddLine(MCGContextRef context, MCGPoint start, MCGPoint finish);
void MCGContextAddPolygon(MCGContextRef context, const MCGPoint *points, uindex_t arity);
void MCGContextAddPolyline(MCGContextRef context, const MCGPoint *points, uindex_t arity);
void MCGContextAddRoundedPolygon(MCGContextRef context, const MCGPoint *points, uindex_t arity);
void MCGContextAddRoundedPolyline(MCGContextRef context, const MCGPoint *points, uindex_t arity);
void MCGContextAddDot(MCGContextRef context, MCGPoint location);
void MCGContextAddPath(MCGContextRef context, MCGPathRef path);

// Path primitives
void MCGContextBeginPath(MCGContextRef context);
void MCGContextMoveTo(MCGContextRef context, MCGPoint end_point);
void MCGContextLineTo(MCGContextRef context, MCGPoint end_point);
void MCGContextQuadraticTo(MCGContextRef context, MCGPoint control_point, MCGPoint end_point);
void MCGContextCubicTo(MCGContextRef context, MCGPoint first_control_point, MCGPoint second_control_point, MCGPoint end_point);
void MCGContextArcTo(MCGContextRef context, MCGSize radii, MCGFloat rotation, bool large_arc, bool sweep, MCGPoint end_point);
void MCGContextCloseSubpath(MCGContextRef context);

// Operations

// Copy the current path as an (immutable) path object. If the context is invalid, or
// the path could not be copied the (empty) error path is returned. Note that failure
// to copy the path does *not* cause the context to become invalid, rather the returned
// path is.
void MCGContextCopyPath(MCGContextRef context, MCGPathRef& r_path);

// Fill the current path using the current paint and fill rule. This discards the path.
void MCGContextFill(MCGContextRef context);
// Thicken the current path using the current stroke attributes and fill the resulting
// path using the non-zero rule. This discards the path.
void MCGContextStroke(MCGContextRef context);
// Fills the current path using the current paint and fill rule. Then thicken the current
// path using the current stroke attributes and fill the resulting path using the non-zero
// rule. This discards the path.
void MCGContextFillAndStroke(MCGContextRef context);
// Intersects the current clipping path with the current path; the inside of the current
// path is determined with the current fill rule. This discards the path.
void MCGContextClip(MCGContextRef context);
// Replace the current path by one thickened using the current stroke attributes.
void MCGContextThicken(MCGContextRef context);
// Replace the current path by one entirely consisting of moveto, lineto and close commands.
void MCGContextFlatten(MCGContextRef context);
// Replace the current path by one consisting of no overlapping subpaths or self
// intersections. Interior is determined by current fill rule.
void MCGContextSimplify(MCGContextRef context);

void MCGContextDrawPixels(MCGContextRef context, const MCGRaster& raster, MCGRectangle dst_rect, MCGImageFilter filter);
void MCGContextDrawImage(MCGContextRef context, MCGImageRef image, MCGRectangle dst_rect, MCGImageFilter filter);
void MCGContextDrawRectOfImage(MCGContextRef self, MCGImageRef p_image, MCGRectangle p_src, MCGRectangle p_dst, MCGImageFilter p_filter);

void MCGContextDrawText(MCGContextRef context, const char* text, uindex_t length, MCGPoint location, uint32_t font_size);
MCGFloat MCGContextMeasureText(MCGContextRef context, const char *text, uindex_t length, uint32_t font_size);

////////////////////////////////////////////////////////////////////////////////

// Transforms
MCGAffineTransform MCGAffineTransformMakeIdentity(void);
MCGAffineTransform MCGAffineTransformMakeRotation(MCGFloat p_angle);
MCGAffineTransform MCGAffineTransformMakeTranslation(MCGFloat p_xoffset, MCGFloat p_yoffset);
MCGAffineTransform MCGAffineTransformMakeScale(MCGFloat p_xscale, MCGFloat p_yscale);

MCGAffineTransform MCGAffineTransformConcat(const MCGAffineTransform& transform_1, const MCGAffineTransform& transform_2);
MCGAffineTransform MCGAffineTransformRotate(const MCGAffineTransform& transform, MCGFloat angle);
MCGAffineTransform MCGAffineTransformTranslate(const MCGAffineTransform& transform, MCGFloat xoffset, MCGFloat yoffset);
MCGAffineTransform MCGAffineTransformScale(const MCGAffineTransform& transform, MCGFloat xscale, MCGFloat yscale);
MCGAffineTransform MCGAffineTransformInvert(const MCGAffineTransform& transform);

MCGPoint MCGPointApplyAffineTransform(const MCGPoint& p_point, const MCGAffineTransform& p_transform);
MCGRectangle MCGRectangleApplyAffineTransform(const MCGRectangle& p_rect, const MCGAffineTransform& p_transform);

////////////////////////////////////////////////////////////////////////////////

#endif