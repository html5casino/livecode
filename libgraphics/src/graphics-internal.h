#ifndef __MC_GRAPHICS_INTERNAL__
#define __MC_GRAPHICS_INTERNAL__

#include "graphics.h"
#include <SkCanvas.h>
#include <SkDashPathEffect.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

////////////////////////////////////////////////////////////////////////////////

typedef struct __MCGPattern *MCGPatternRef;

struct __MCGPattern
{
	MCGImageRef			pattern;
	MCGAffineTransform	transform;
	MCGImageFilter		filter;
	uint32_t			references;
};

bool MCGPatternCreate(MCGImageRef image, MCGAffineTransform transform, MCGImageFilter filter, MCGPatternRef& r_pattern);
MCGPatternRef MCGPatternRetain(MCGPatternRef pattern);
void MCGPatternRelease(MCGPatternRef pattern);
bool MCGPatternToSkShader(MCGPatternRef pattern, SkShader*& r_shader);

////////////////////////////////////////////////////////////////////////////////

typedef struct __MCGGradient *MCGGradientRef;

struct __MCGGradient
{
	MCGGradientFunction	function;	
	MCGColor			*colors;
	MCGFloat			*stops;
	uindex_t			ramp_length;	
	bool				mirror;
	bool				wrap;
	uint32_t			repeats;	
	MCGAffineTransform	transform;
	MCGImageFilter		filter;	
	uint32_t			references;
};

bool MCGGradientCreate(MCGGradientFunction function, const MCGFloat* stops, const MCGColor* colors, uindex_t ramp_length, bool mirror, bool wrap, uint32_t repeats, MCGAffineTransform transform, MCGImageFilter filter, MCGGradientRef& r_gradient);
MCGGradientRef MCGGradientRetain(MCGGradientRef gradient);
void MCGGradientRelease(MCGGradientRef gradient);
bool MCGGradientToSkShader(MCGGradientRef gradient, SkShader*& r_shader);

////////////////////////////////////////////////////////////////////////////////

typedef struct __MCGContextState *MCGContextStateRef;

struct __MCGContextState
{
	MCGFloat			opacity;
	MCGBlendMode		blend_mode;
	MCGFloat			flatness;
	bool				should_antialias;
	
	MCGColor			fill_color;
	MCGFloat			fill_opacity;
	MCGFillRule			fill_rule;
	MCGPatternRef		fill_pattern;
	MCGGradientRef		fill_gradient;
	
	MCGColor			stroke_color;
	MCGFloat			stroke_opacity;
	MCGStrokeAttr		stroke_attr;
	MCGPatternRef		stroke_pattern;
	MCGGradientRef		stroke_gradient;
	
	bool				is_layer_begin_pt;
	MCGContextStateRef	parent;
};

////////////////////////////////////////////////////////////////////////////////

struct __MCGImage
{
	SkBitmap	*bitmap;
	bool		is_valid;
	uint32_t	references;
};

struct __MCGPath
{
	SkPath		*path;
	bool		is_valid;
	bool		is_mutable;
	uint32_t	references;
};

struct __MCGContext
{
	SkCanvas			*canvas;
	
	uint32_t			width;
	uint32_t			height;
	MCGContextStateRef	state;
	MCGPathRef			path;
	
	bool				is_valid;
	uint32_t			references;
};

////////////////////////////////////////////////////////////////////////////////

struct __MCGDashes
{
	MCGFloat	phase;
	MCGFloat	*lengths;
	uindex_t	count;
	uint32_t	references;
};

bool MCGDashesCreate(MCGFloat phase, const MCGFloat *lengths, uindex_t arity, MCGDashesRef& r_dashes);
MCGDashesRef MCGDashesRetain(MCGDashesRef dashes);
void MCGDashesRelease(MCGDashesRef dashes);
bool MCGDashesToSkDashPathEffect(MCGDashesRef dashes, SkDashPathEffect*& r_path_effect);

////////////////////////////////////////////////////////////////////////////////

inline bool MCGRectangleIntersects(MCGRectangle p_rect_1, MCGRectangle p_rect_2)
{
	return (p_rect_1 . origin . x < (p_rect_2 . origin . x +  p_rect_2 . size . width)) && 
	(p_rect_2 . origin . x < (p_rect_1 . origin . x +  p_rect_1 . size . width)) &&
	(p_rect_1 . origin . y < (p_rect_2 . origin . y +  p_rect_2 . size . height)) &&
	(p_rect_2 . origin . y < (p_rect_1 . origin . y +  p_rect_1 . size . height));	
}

////////////////////////////////////////////////////////////////////////////////

enum MCGPixelOwnershipType
{
	kMCGPixelOwnershipTypeBorrow,
	kMCGPixelOwnershipTypeTake,
	kMCGPixelOwnershipTypeCopy,
};

bool MCGRasterToSkBitmap(const MCGRaster& raster, MCGPixelOwnershipType p_ownership, SkBitmap& bitmap);
SkXfermode* MCGBlendModeToSkXfermode(MCGBlendMode mode);
MCGBlendMode MCGBlendModeToSkXfermode(SkXfermode* mode);
void MCGAffineTransformToSkMatrix(const MCGAffineTransform& transform, SkMatrix& r_matrix);

////////////////////////////////////////////////////////////////////////////////

inline MCGColor MCGColorMakeRGBA(MCGFloat p_red, MCGFloat p_green, MCGFloat p_blue, MCGFloat p_alpha)
{	
	return ((uint8_t)(p_red * 255) << 16) | ((uint8_t)(p_green * 255) << 8) | ((uint8_t)(p_blue * 255) << 0) | ((uint8_t)(p_alpha * 255) << 24);
}

inline void MCGColorSetRed(MCGColor& x_color, MCGFloat p_red) {
	x_color = (x_color & 0xFF00FFFF) | ((uint8_t)(p_red * 255) << 16);
}

inline void MCGColorSetGreen(MCGColor& x_color, MCGFloat p_green) {
	x_color = (x_color & 0xFFFF00FF) | ((uint8_t)(p_green * 255) << 8);
}

inline void MCGColorSetBlue(MCGColor& x_color, MCGFloat p_blue) {
	x_color = (x_color & 0xFFFFFF00) | ((uint8_t)(p_blue * 255) << 0);
}

inline void MCGColorSetAlpha(MCGColor& x_color, MCGFloat p_alpha) {
	x_color = (x_color & 0x00FFFFFF) | ((uint8_t)(p_alpha * 255) << 24);
}

inline MCGFloat MCGColorGetRed(MCGColor p_color) {
    return ((p_color >> 16) & 0xFF) / 255.0f;
}

inline MCGFloat MCGColorGetGreen(MCGColor p_color) {
    return ((p_color >> 8) & 0xFF) / 255.0f;
}

inline MCGFloat MCGColorGetBlue(MCGColor p_color) {
    return ((p_color >> 0) & 0xFF) / 255.0f;
}

inline MCGFloat MCGColorGetAlpha(MCGColor p_color) {
    return ((p_color >> 24) & 0xFF) / 255.0f;
}

inline SkColor MCGColorToSkColor(MCGColor p_color)
{
	return p_color;
}

inline MCGColor MCGColorFromSkColor(SkColor p_color)
{
	return p_color;
}

////////////////////////////////////////////////////////////////////////////////

inline MCGFloat MCGFloatDegreesToRadians(MCGFloat p_degrees)
{
	return p_degrees * M_PI / 180;
}

inline MCGFloat MCGFloatDegreesFromRadians(MCGFloat p_radians)
{
	return p_radians * 180 / M_PI;
}

inline MCGFloat MCGFloatCos(MCGFloat p_angle)
{
	return cosf(p_angle);
}

inline MCGFloat MCGFloatSin(MCGFloat p_angle)
{
	return sinf(p_angle);
}

////////////////////////////////////////////////////////////////////////////////

inline SkScalar MCGFloatToSkScalar(MCGFloat p_ploat)
{
	return p_ploat;
}

inline MCGFloat MCGFloatFromSkScalar(SkScalar p_scalar)
{
	return p_scalar;
}

inline SkScalar MCGCoordToSkCoord(MCGFloat p_coord)
{
	// TODO: Shift coordinate appropriately 
	return p_coord;
}

inline MCGFloat MCGCoordFromSkCoord(SkScalar p_coord)
{
	// TODO: Shift coordinate appropriately 
	return p_coord;
}

inline SkPaint::Join MCGJoinStyleToSkJoinStyle(MCGJoinStyle p_style)
{
	switch (p_style)
	{
		case kMCGJoinStyleBevel:
			return SkPaint::kBevel_Join;
		case kMCGJoinStyleRound:
			return SkPaint::kRound_Join;
		case kMCGJoinStyleMiter:
			return SkPaint::kMiter_Join;			
	}
}

inline MCGJoinStyle MCGJoinStyleFromSkJoinStyle(SkPaint::Join p_style)
{
	switch (p_style)
	{
		case SkPaint::kBevel_Join:
			return kMCGJoinStyleBevel;
		case SkPaint::kRound_Join:
			return kMCGJoinStyleRound;
		case SkPaint::kMiter_Join:
		default:
			return kMCGJoinStyleMiter;
	}
}

inline SkPaint::Cap MCGCapStyleToSkCapStyle(MCGCapStyle p_style)
{
	switch (p_style)
	{
		case kMCGCapStyleButt:
			return SkPaint::kButt_Cap;
		case kMCGCapStyleRound:
			return SkPaint::kRound_Cap;
		case kMCGCapStyleSquare:
			return SkPaint::kSquare_Cap;
	}
}

inline MCGCapStyle MCGCapStyleFromSkCapStyle(SkPaint::Cap p_style)
{
	switch (p_style)
	{
		case SkPaint::kRound_Cap:
			return kMCGCapStyleRound;
		case SkPaint::kSquare_Cap:
			return kMCGCapStyleSquare;
		default:
		case SkPaint::kButt_Cap:
			return kMCGCapStyleButt;			
	}
}

inline SkRect MCGRectangleToSkRect(MCGRectangle p_rect)
{
	return SkRect::MakeXYWH(MCGCoordToSkCoord(p_rect . origin . x), MCGCoordToSkCoord(p_rect . origin . y), MCGFloatToSkScalar(p_rect . size . width), MCGFloatToSkScalar(p_rect . size . height));
}

inline MCGRectangle MCGRectangleFromSkRect(SkRect p_rect)
{
	return MCGRectangleMake(MCGCoordFromSkCoord(p_rect . x()), MCGCoordFromSkCoord(p_rect . y()), p_rect . width(), p_rect . height());
}

inline SkPoint MCGPointToSkPoint(MCGPoint p_point)
{
	return SkPoint::Make(MCGCoordToSkCoord(p_point . x), MCGCoordToSkCoord(p_point . y));
}

inline MCGPoint MCGPointFromSkPoint(SkPoint p_point)
{
	MCGPoint t_point;
	t_point . x = MCGCoordFromSkCoord(p_point . x());
	t_point . y = MCGCoordFromSkCoord(p_point . y());
	return t_point;
}

inline SkPath::FillType MCGFillRuleToSkFillType(MCGFillRule p_rule)
{
	switch (p_rule)
	{
		case kMCGFillRuleNonZero:
			return SkPath::kWinding_FillType;
		case kMCGFillRuleEvenOdd:
			return SkPath::kEvenOdd_FillType;
	}
}

inline MCGFillRule MCGFillRuleToSkFillType(SkPath::FillType p_fill_type)
{
	switch (p_fill_type)
	{
		case SkPath::kWinding_FillType:
			return kMCGFillRuleNonZero;
		case SkPath::kEvenOdd_FillType:
		default:
			return kMCGFillRuleEvenOdd;
	}
}

////////////////////////////////////////////////////////////////////////////////

inline SkBitmap::Config MCGRasterFormatToSkBitmapConfig(MCGRasterFormat p_format)
{
	switch (p_format)
	{
	case kMCGRasterFormat_A:
		return SkBitmap::kA8_Config;
	case kMCGRasterFormat_ARGB:
	case kMCGRasterFormat_U_ARGB:
	case kMCGRasterFormat_xRGB:
		return SkBitmap::kARGB_8888_Config;
	}
}

inline MCGRasterFormat MCGRasterFormatFromSkBitmapConfig(SkBitmap::Config p_config)
{
	switch (p_config)
	{
	case SkBitmap::kA8_Config:
		return kMCGRasterFormat_A;
	case SkBitmap::kARGB_8888_Config:
		return kMCGRasterFormat_ARGB;
	}
}

////////////////////////////////////////////////////////////////////////////////

#endif