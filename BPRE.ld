/*
 * FireRed US 1.0 (BPRE) symbols used by the Oak intro.  Same idea as CFRU's BPRE.ld:
 * every ROM function/variable the C code touches is named here ONCE and declared with
 * a real prototype in include/.  Functions are Thumb, so the address has bit 0 set.
 *
 * Names follow the pokefirered decompilation where the match is certain.  Entries marked
 * [?] are carried over from the original OakTutorial with a descriptive name only; the
 * address is right (the old project called it) but the decomp name was not verified.
 * docs/SYMBOLS.md lists old name -> new name.
 */

/* ----- RAM ------------------------------------------------------------------------ */
gMain                      = 0x030030F0;
gTasks                     = 0x03005090;
gSaveBlock1Ptr             = 0x03005008;
gSaveBlock2Ptr             = 0x0300500C;
gSprites                   = 0x0202063C;
gPaletteFade               = 0x02037AB8;
gStringVar4                = 0x02021D18;

/* ----- ROM data ------------------------------------------------------------------- */
gDummySpriteAnimTable        = 0x08231CF0;
gDummySpriteAffineAnimTable  = 0x08231CFC;
gUnk_082346F8                = 0x082346F8;   /* [?] image table the original used for the Pokemon sprite */
gUnk_08234944                = 0x08234944;   /* [?] affine anim table the original used for the Pokemon sprite */
gMonFrontPicTable            = 0x082350AC;
gMonPaletteTable             = 0x0823730C;
gBallSpriteSheets            = 0x0826056C;
gBallSpritePalettes          = 0x082605CC;
gBallOamData                 = 0x0826062C;
gOakIntroBgTemplates         = 0x08462E58;   /* [?] BG template table of the vanilla Oak speech */

/* ----- main / tasks / sprites ----------------------------------------------------- */
SetMainCallback2           = 0x08000545;
CreateTask                 = 0x0807741D;
CreateSprite               = 0x08006F8D;
DestroySpriteAndFreeResources = 0x08007805;
LoadSpritePalette          = 0x08008929;
LoadCompressedSpriteSheet  = 0x0800EBCD;
LoadCompressedSpritePalette = 0x0800F079;
SetGpuReg                  = 0x08000A39;
sub_8006B10                = 0x08006B11;   /* [?] reset sprite data */
sub_80088F0                = 0x080088F1;   /* [?] free all sprite palettes */
sub_8000B68                = 0x08000B69;   /* [?] called with 1 at the end of CB2 setup */
Random                     = 0x08044EC9;

/* ----- palette fade / sound ------------------------------------------------------- */
BeginNormalPaletteFade     = 0x08070589;
LoadPalette                = 0x080703ED;
sub_8070528                = 0x08070529;   /* [?] reset palette fade */
sub_8087E64                = 0x08087E65;   /* [?] */
PlayBGM                    = 0x080722A1;
PlaySE                     = 0x080722CD;
FadeOutBGM                 = 0x08071D65;

/* ----- BG / graphics -------------------------------------------------------------- */
ResetBgsAndClearDma3BusyFlags = 0x08001619;
InitBgsFromTemplates       = 0x08001659;
SetBgTilemapBuffer         = 0x08001FA1;
ChangeBgX                  = 0x08001B91;
ChangeBgY                  = 0x08001D09;
GetBgX                     = 0x08001CCD;
GetBgY                     = 0x08001E45;
ShowBg                     = 0x080019BD;
HideBg                     = 0x080019D1;
LoadBgTiles                = 0x080017D1;
CopyToBgTilemapBuffer      = 0x08002041;
CopyBgTilemapBufferToVram  = 0x080020BD;
LZ77UnCompVram             = 0x081E3B6D;
malloc_and_decompress      = 0x080F6AA1;   /* LZ77 -> new heap block, size returned via pointer */
AllocZeroed                = 0x08002BB1;
Free                       = 0x08002BC5;
memcpy                     = 0x081E5E79;
memset                     = 0x081E5ED9;

/* ----- windows / text / menus ----------------------------------------------------- */
AddWindow                  = 0x08003CE5;
RemoveWindow               = 0x08003E3D;
PutWindowTilemap           = 0x08003FA1;
FillWindowPixelBuffer      = 0x0800445D;
CopyWindowToVram           = 0x08003F21;
IsTextPrinterActive        = 0x08002E65;
StringExpandPlaceholders   = 0x08008FCD;
StringCopy                 = 0x08008D85;
LoadMessageBox             = 0x080F6EE5;   /* [?] draws the dialogue frame for window 0 */
AddTextPrinterParameterized2 = 0x080F6CD1;
HideMessageBox             = 0x080694F5;   /* [?] closes the dialogue box */
GetFontAttribute           = 0x080F79D9;
OakMenu_PrintText          = 0x0812E51D;   /* [?] (window, font, x, y, a, b, str) */
DrawStdFrameWithCustomTileAndPalette = 0x0810F2E9;
ClearStdWindowAndFrame     = 0x0810F4D9;
ClearDialogWindowAndFrame  = 0x080F6F55;
Menu_InitCursor            = 0x0810F7D9;
Menu_ProcessInputNoWrapClearOnChoose = 0x0810FA05;
sub_8003ECC                = 0x08003ECD;   /* [?] window system reset */
sub_80F6C6C                = 0x080F6C6D;   /* [?] */
sub_80F6C98                = 0x080F6C99;   /* [?] */
sub_80F6808                = 0x080F6809;   /* [?] */
DoNamingScreen             = 0x0809D955;

/* ----- helpers from the vanilla Oak speech that the original project reused ------- */
VBlankCB_OakSpeech         = 0x0812EAFD;   /* [?] */
CB2_OakSpeech              = 0x0812EB11;   /* [?] */
Task_OakSpeech_Continue    = 0x08130915;   /* [?] task the new-game flow continues with after the speech */
OakSpeech_FadeBgOut        = 0x0813144D;   /* [?] (taskId, bg) fades a BG layer out */
OakSpeech_FadeBgIn         = 0x081315CD;   /* [?] (taskId, bg) fades a BG layer in  */
sub_804B8C8                = 0x0804B8C9;   /* [?] ball-open particles (x, y, 0, 0) */
sub_804B8F0                = 0x0804B8F1;   /* [?] ball-open glow/flash (1, 0, 0xFF) */
