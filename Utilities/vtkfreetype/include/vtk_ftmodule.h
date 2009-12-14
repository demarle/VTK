/*
 *  VTK_FREETYPE_CHANGE this file is new for VTK.
 *
 *  This is used to override the default module headers.
 *
 *  This file is based on freetype's include/freetype/config/ftmodule.h.
 *  Be sure to keep it in sync with the original file when updating freetype
 *
 *  Please read `docs/CUSTOMIZE' section IV-3 for more information.
 *
 */

/* VTK_FREETYPE_CHANGE removed the line below */
/* FT_USE_MODULE( FT_Module_Class, autofit_module_class ) */
FT_USE_MODULE( FT_Driver_ClassRec, tt_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, t1_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, cff_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, t1cid_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, pfr_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, t42_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, winfnt_driver_class )
FT_USE_MODULE( FT_Driver_ClassRec, pcf_driver_class )
FT_USE_MODULE( FT_Module_Class, psaux_module_class )
FT_USE_MODULE( FT_Module_Class, psnames_module_class )
FT_USE_MODULE( FT_Module_Class, pshinter_module_class )
FT_USE_MODULE( FT_Renderer_Class, ft_raster1_renderer_class )
FT_USE_MODULE( FT_Module_Class, sfnt_module_class )
FT_USE_MODULE( FT_Renderer_Class, ft_smooth_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_smooth_lcd_renderer_class )
FT_USE_MODULE( FT_Renderer_Class, ft_smooth_lcdv_renderer_class )
FT_USE_MODULE( FT_Driver_ClassRec, bdf_driver_class )

/* EOF */
