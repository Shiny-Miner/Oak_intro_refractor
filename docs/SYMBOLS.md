# Old OakTutorial name -> new name (FireRed US 1.0 addresses, Thumb bit set)

| address | old name | new name | note |
|---|---|---|---|
| 0x08000545 | set_callback2 | SetMainCallback2 | |
| 0x0807741D | task_add | CreateTask | |
| 0x08006F8D | object_display | CreateSprite | |
| 0x08007805 | object_delete_and_free | DestroySpriteAndFreeResources | |
| 0x0800EBCD | object_load_compressed_graphics | LoadCompressedSpriteSheet | |
| 0x08008929 | object_load_palette | LoadSpritePalette | old struct had the tag twice |
| 0x08000A39 | display_ioreg_set | SetGpuReg | |
| 0x08070589 | fadescreen | BeginNormalPaletteFade | |
| 0x080703ED | gpu_pal_apply | LoadPalette | |
| 0x080722A1 / 0x080722CD | song_play_for_text / audio_play | PlayBGM / PlaySE | |
| 0x08071D65 | (cast) | FadeOutBGM | |
| 0x08001619 / 0x08001659 | gpu_tile_bg_drop_all_sets / bg_vram_setup | ResetBgsAndClearDma3BusyFlags / InitBgsFromTemplates | |
| 0x08001FA1 | bgid_set_tilemap | SetBgTilemapBuffer | |
| 0x08001B91 / 0x08001D09 / 0x08001CCD | bgid_mod_x/y_offset, bgid_get_x_offset | ChangeBgX / ChangeBgY / GetBgX | |
| 0x080019BD / 0x080019D1 | gpu_sync_bg_show / hide | ShowBg / HideBg | |
| 0x080017D1 / 0x08002041 / 0x080020BD | gpu_copy_to_tileset / gpu_copy_tilemap / bgid_send_tilemap | LoadBgTiles / CopyToBgTilemapBuffer / CopyBgTilemapBufferToVram | tilemap mode 0 = LZ77 source |
| 0x080F6AA1 | malloc_and_LZ77UnComp | malloc_and_decompress | |
| 0x08002BB1 / 0x08002BC5 | malloc_and_clear / free | AllocZeroed / Free | |
| 0x08003CE5 | load_rbox | AddWindow | |
| 0x08003FA1 / 0x0800445D / 0x08003F21 | fix_box / clear_box / rboxid_to_vram | PutWindowTilemap / FillWindowPixelBuffer / CopyWindowToVram | |
| 0x08003E3D / 0x0810F4D9 / 0x080F6F55 | erase_box / free_box / destroy_box | RemoveWindow / ClearStdWindowAndFrame / ClearDialogWindowAndFrame | old called free_box(1) with 1 arg; now (windowId, TRUE) |
| 0x08002E65 | check_a_pressed | IsTextPrinterActive | |
| 0x08008FCD | fdecoder | StringExpandPlaceholders | |
| 0x080F6CD1 | box_related_one | AddTextPrinterParameterized2 | |
| 0x0810F2E9 | outline_box | DrawStdFrameWithCustomTileAndPalette | |
| 0x080F79D9 / 0x0810F7D9 | multichoice_field / multichoice_prepare | GetFontAttribute / Menu_InitCursor | |
| 0x0810FA05 | multichoice_get_response | Menu_ProcessInputNoWrapClearOnChoose | |
| 0x0809D955 | name_screen | DoNamingScreen | |
| 0x0812EAFD / 0x0812EB11 | (cast) | VBlankCB_OakSpeech / CB2_OakSpeech | [?] vanilla Oak speech loop |
| 0x08130915 | (cast) | Task_OakSpeech_Continue | [?] where the new-game flow resumes |

Calls from the old code that the rewrite deliberately does not make: `0x08131169(index, 1)` (unknown purpose,
called with garbage state every frame while the naming screen ran) and `0x0813144C / 0x081315CC`
(vanilla BG fade tasks, only used by the unreachable professor scenes). They are still in `BPRE.ld`.
