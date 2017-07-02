/*
 * Parameter handler.
 */

#ifndef _param_h_
#define _param_h_
s32 PARAM_Init(u32 mode);
s32 PARAM_ButtonHandle(u32 button_mask);
s32 PARAM_EncoderChange(u32 encoder, u32 delta);
s32 PARAM_Update_1mS(void);

#endif /* _param_h_ */
