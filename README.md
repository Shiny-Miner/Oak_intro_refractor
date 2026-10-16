# Oak_intro_refractor

A rewrite of OakTutorial into Sinnoh region's Roman's intro. Made by touched and mordern rewritten by me(Shiny Miner).



https://github.com/user-attachments/assets/797bb76f-4072-402c-b776-782074cf778f



```
BPRE.ld            every ROM function/variable the code uses, named once
linker.ld          generated (OFFSET_TO_PUT)
hooks              <rom address> <function> [reg]  – vanilla function -> your function
bytereplacement    <rom address> <hex bytes>       – raw patches
include/           headers with real prototypes (no more implicit declarations)
src/               the intro (see below)
assembly/          bios.s (BitUnPack swi)
strings/*.string   text: `label: text`, converted at build time
graphics/*.png     drop an image in, it gets converted (no grit)
scripts/make.py    build + insert, writes test.gba and offsets.ini
scripts/clean.py   remove build output
docs/SYMBOLS.md    old OakTutorial name -> new name
```

## Build

Same build as CFRU, use
```python scripts//make.py```

## Adding things

* **Tunables** (fade speed, positions, music, species changeable in) – `include/config.h`.

## Source map

| file | what |
|---|---|
| `oak_intro.c` | hook entry, video setup state machine (also the naming-screen return callback), hello scene (then Rowan fades in) |
| `oak_intro_rowan.c` | Rowan's speech and advice menu |
| `oak_intro_ball.c` | Poké Ball roll, Pokémon hop |
| `oak_intro_gender.c` | boy/girl chooser |
| `oak_intro_name.c` | player name, professor introduces rival, rival name |
| `oak_intro_finish.c` | goodbye, shrink, hand-over |
| `oak_intro_actor.c` | 64x128 characters (two stacked sprites), swapping the centre character |
| `oak_intro_ui.c` | message box, multichoice menus, sprite fade, screen fades |

