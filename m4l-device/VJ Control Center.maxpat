{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 0,
   "revision": 0,
   "architecture": "x64",
   "modernui": 1
  },
  "classnamespace": "dsp.audioeffect",
  "rect": [
   60.0,
   60.0,
   1560.0,
   900.0
  ],
  "bglocked": 0,
  "openinpresentation": 1,
  "openrect": [
   0.0,
   0.0,
   300.0,
   210.0
  ],
  "default_fontsize": 12.0,
  "default_fontface": 0,
  "default_fontname": "Arial",
  "gridonopen": 1,
  "gridsize": [
   15.0,
   15.0
  ],
  "gridsnaponopen": 1,
  "objectsnaponopen": 1,
  "statusbarvisible": 2,
  "toolbarvisible": 1,
  "boxanimatetime": 200,
  "enablehscroll": 1,
  "enablevscroll": 1,
  "devicewidth": 1000.0,
  "description": "",
  "digest": "",
  "tags": "",
  "editionbox": [
   0.0,
   0.0,
   0.0,
   0.0
  ],
  "showontab": 0,
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "newobj",
     "text": "plugin~",
     "numinlets": 0,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      "signal"
     ],
     "patching_rect": [
      320.0,
      20.0,
      80.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-60",
     "maxclass": "newobj",
     "text": "plugout~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      320.0,
      720.0,
      80.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "newobj",
     "text": "+~",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      320.0,
      60.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "newobj",
     "text": "onepole~ 150",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      40.0,
      110.0,
      100.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-4",
     "maxclass": "newobj",
     "text": "onepole~ 2000",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      220.0,
      110.0,
      100.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "newobj",
     "text": "abs~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      600.0,
      110.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "newobj",
     "text": "-~",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      220.0,
      160.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "newobj",
     "text": "-~",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      400.0,
      160.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "newobj",
     "text": "abs~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      40.0,
      160.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "newobj",
     "text": "abs~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      220.0,
      210.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "newobj",
     "text": "abs~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      400.0,
      210.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "newobj",
     "text": "average~ 200",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      600.0,
      160.0,
      90.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "newobj",
     "text": "average~ 200",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      40.0,
      210.0,
      90.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "newobj",
     "text": "average~ 200",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      220.0,
      260.0,
      90.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "newobj",
     "text": "average~ 200",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ],
     "patching_rect": [
      400.0,
      260.0,
      90.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "toggle",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      780.0,
      20.0,
      20.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      244.0,
      34.0,
      20.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "newobj",
     "text": "qmetro 33",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      780.0,
      60.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "newobj",
     "text": "t b b b b b",
     "numinlets": 1,
     "numoutlets": 5,
     "outlettype": [
      "bang",
      "bang",
      "bang",
      "bang",
      "bang"
     ],
     "patching_rect": [
      780.0,
      100.0,
      90.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "newobj",
     "text": "snapshot~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      600.0,
      210.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "newobj",
     "text": "snapshot~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      40.0,
      260.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "newobj",
     "text": "snapshot~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      220.0,
      310.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "newobj",
     "text": "snapshot~",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      400.0,
      310.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "newobj",
     "text": "* 3.",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      600.0,
      260.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "newobj",
     "text": "* 4.",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40.0,
      310.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "newobj",
     "text": "* 4.",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      220.0,
      360.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "newobj",
     "text": "* 6.",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      400.0,
      360.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "newobj",
     "text": "clip 0. 1.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      600.0,
      310.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "newobj",
     "text": "clip 0. 1.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40.0,
      360.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "newobj",
     "text": "clip 0. 1.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      220.0,
      410.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "newobj",
     "text": "clip 0. 1.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      400.0,
      410.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      700.0,
      310.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-36",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      130.0,
      360.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-37",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      320.0,
      410.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-38",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      500.0,
      410.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-30",
     "maxclass": "newobj",
     "text": "prepend /audio/level",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      600.0,
      360.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "newobj",
     "text": "prepend /audio/bass",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      40.0,
      410.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "newobj",
     "text": "prepend /audio/mid",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      220.0,
      460.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "newobj",
     "text": "prepend /audio/high",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      400.0,
      460.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-42",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      600.0,
      410.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      40.0,
      460.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      220.0,
      510.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-45",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      400.0,
      510.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-40",
     "maxclass": "newobj",
     "text": "OpenSoundControl",
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "",
      "",
      ""
     ],
     "patching_rect": [
      300.0,
      580.0,
      130.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "newobj",
     "text": "udpsend 127.0.0.1 9000",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      300.0,
      630.0,
      180.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-46",
     "maxclass": "newobj",
     "text": "loadbang",
     "numinlets": 0,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      900.0,
      20.0,
      70.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-47",
     "maxclass": "message",
     "text": "resetallthewaymode 1",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      750.0,
      410.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-48",
     "maxclass": "message",
     "text": "errorreporting 1",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      750.0,
      460.0,
      120.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-63",
     "maxclass": "message",
     "text": "1",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      780.0,
      460.0,
      30.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-71",
     "maxclass": "newobj",
     "text": "live.path live_set",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      900.0,
      60.0,
      110.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-72",
     "maxclass": "newobj",
     "text": "live.object",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      900.0,
      100.0,
      80.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-73",
     "maxclass": "message",
     "text": "get current_song_time",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      900.0,
      140.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-74",
     "maxclass": "newobj",
     "text": "route current_song_time",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      ""
     ],
     "patching_rect": [
      900.0,
      180.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-75",
     "maxclass": "newobj",
     "text": "expr $f1 - floor($f1)",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      900.0,
      220.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-76",
     "maxclass": "newobj",
     "text": "prepend /audio/beatphase",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      900.0,
      260.0,
      170.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-77",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      900.0,
      300.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-78",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ],
     "patching_rect": [
      1000.0,
      220.0,
      60.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      174.0,
      34.0,
      50.0,
      18.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-90",
     "maxclass": "comment",
     "text": "VJ CONTROL CENTER",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      40.0,
      750.0,
      250.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      2.0,
      260.0,
      16.0
     ],
     "textcolor": [
      1.0,
      1.0,
      1.0,
      1.0
     ],
     "fontsize": 13.0,
     "fontface": 1
    }
   },
   {
    "box": {
     "id": "obj-91",
     "maxclass": "comment",
     "text": "Level",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      40.0,
      780.0,
      40.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      14.0,
      20.0,
      40.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-92",
     "maxclass": "live.meter~",
     "numinlets": 1,
     "numoutlets": 0,
     "parameter_enable": 0,
     "patching_rect": [
      40.0,
      800.0,
      18.0,
      110.0
     ],
     "presentation": 1,
     "presentation_rect": [
      14.0,
      34.0,
      18.0,
      76.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-93",
     "maxclass": "comment",
     "text": "Bass",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      90.0,
      780.0,
      40.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      54.0,
      20.0,
      40.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-94",
     "maxclass": "live.meter~",
     "numinlets": 1,
     "numoutlets": 0,
     "parameter_enable": 0,
     "patching_rect": [
      90.0,
      800.0,
      18.0,
      110.0
     ],
     "presentation": 1,
     "presentation_rect": [
      54.0,
      34.0,
      18.0,
      76.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-95",
     "maxclass": "comment",
     "text": "Mid",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      140.0,
      780.0,
      40.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      94.0,
      20.0,
      40.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-96",
     "maxclass": "live.meter~",
     "numinlets": 1,
     "numoutlets": 0,
     "parameter_enable": 0,
     "patching_rect": [
      140.0,
      800.0,
      18.0,
      110.0
     ],
     "presentation": 1,
     "presentation_rect": [
      94.0,
      34.0,
      18.0,
      76.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-97",
     "maxclass": "comment",
     "text": "High",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      190.0,
      780.0,
      40.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      134.0,
      20.0,
      40.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-98",
     "maxclass": "live.meter~",
     "numinlets": 1,
     "numoutlets": 0,
     "parameter_enable": 0,
     "patching_rect": [
      190.0,
      800.0,
      18.0,
      110.0
     ],
     "presentation": 1,
     "presentation_rect": [
      134.0,
      34.0,
      18.0,
      76.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-99",
     "maxclass": "comment",
     "text": "Beat",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      240.0,
      780.0,
      40.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      174.0,
      20.0,
      50.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-102",
     "maxclass": "comment",
     "text": "Freeze",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      290.0,
      780.0,
      50.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      240.0,
      20.0,
      50.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-39",
     "maxclass": "comment",
     "text": "VJ Control Center - single M4L device for the VJ Engine: audio analysis (level/bass/mid/high/beatphase/onset), preset dropdown + Next/Prev, scene-name preset triggers, Show Display (brings VJ Engine.exe to front / launches it), and 8 FX knobs sending /effect/param <stage> <param> <scaled value> over OSC to 127.0.0.1:9000 (only affects whichever preset is currently active in the engine). FX1-5 are dedicated to preset 04 'Video Glitch Chain' (RGB Shift, BG Intensity, Block Count, Jitter Amt, Line Density - the 5 real audio-reactive params on that chain's 3 stages) and work immediately; FX6-8 are free/generic (type a Param name + Stage/Min/Max above the knob to repurpose one - stage defaults to -1, a safe no-op, until you do). Audio passes through unaffected (plugin~ -> plugout~).",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      40.0,
      690.0,
      900.0,
      45.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-110",
     "maxclass": "comment",
     "text": "Preset Select",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      1200.0,
      20.0,
      150.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-111",
     "maxclass": "live.numbox",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "bang"
     ],
     "parameter_enable": 1,
     "parameter_longname": "Preset Select",
     "parameter_shortname": "Preset",
     "parameter_type": 0,
     "parameter_mmin": 0.0,
     "parameter_mmax": 31.0,
     "parameter_unitstyle": 0,
     "patching_rect": [
      1200.0,
      60.0,
      50.0,
      22.0
     ],
     "presentation": 0,
     "presentation_rect": [
      14.0,
      175.0,
      50.0,
      20.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Preset Select",
       "parameter_shortname": "Preset",
       "parameter_type": 0,
       "parameter_mmin": 0.0,
       "parameter_mmax": 31.0,
       "parameter_unitstyle": 0,
       "parameter_enum": []
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-112",
     "maxclass": "newobj",
     "text": "prepend /preset/select",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1200.0,
      100.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-113",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      1200.0,
      140.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-114",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      1280.0,
      60.0,
      20.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      172.0,
      134.0,
      20.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-115",
     "maxclass": "message",
     "text": "/preset/next",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1280.0,
      100.0,
      110.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-116",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      1280.0,
      140.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-120",
     "maxclass": "comment",
     "text": "Next",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      1280.0,
      40.0,
      40.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      172.0,
      118.0,
      40.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-117",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      1390.0,
      60.0,
      20.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      202.0,
      134.0,
      20.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-118",
     "maxclass": "message",
     "text": "/preset/previous",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1390.0,
      100.0,
      145.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-119",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      1390.0,
      140.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-121",
     "maxclass": "comment",
     "text": "Prev",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      1390.0,
      40.0,
      40.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      202.0,
      118.0,
      40.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-122",
     "maxclass": "comment",
     "text": "Preset",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      1200.0,
      40.0,
      60.0,
      14.0
     ],
     "presentation": 1,
     "presentation_rect": [
      14.0,
      118.0,
      60.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-123",
     "maxclass": "comment",
     "text": "Onset (Bass Transient) - drives a decaying glitch pulse in the VJ Engine",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      1200.0,
      220.0,
      300.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-124",
     "maxclass": "newobj",
     "text": "> 0.3",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1200.0,
      250.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-125",
     "maxclass": "newobj",
     "text": "change",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1200.0,
      290.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-126",
     "maxclass": "newobj",
     "text": "select 1",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "bang"
     ],
     "patching_rect": [
      1200.0,
      330.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-127",
     "maxclass": "message",
     "text": "/audio/onset",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1200.0,
      370.0,
      100.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-128",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      1200.0,
      410.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-214",
     "maxclass": "comment",
     "text": "Scene -> Preset: name a Session View scene starting with a number (e.g. \"3 Drop\") - selecting it should send /preset/select <n> to the VJ Engine automatically. Wiring mirrors the proven beatphase live.path/live.object pattern above. The Max trial-block that previously stopped new objects from executing was resolved 25.07.2026 (open Max via Live's device Edit button, not standalone, to keep the Max-for-Live authorization) - a fresh loadbang->print chain was confirmed firing after that fix. This scene-link chain itself still needs a hands-on check inside a real Live Session View (requires actually selecting scenes, which can't be driven by OSC/script).",
     "numinlets": 1,
     "numoutlets": 0,
     "outlettype": [],
     "patching_rect": [
      1500.0,
      0.0,
      460.0,
      60.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-201",
     "maxclass": "newobj",
     "text": "live.path live_set view",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1500.0,
      40.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-202",
     "maxclass": "newobj",
     "text": "live.object",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1500.0,
      80.0,
      80.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-203",
     "maxclass": "newobj",
     "text": "qmetro 200",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      1660.0,
      40.0,
      80.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-204",
     "maxclass": "message",
     "text": "get selected_scene",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1660.0,
      80.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-205",
     "maxclass": "newobj",
     "text": "route selected_scene",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      ""
     ],
     "patching_rect": [
      1500.0,
      120.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-206",
     "maxclass": "message",
     "text": "id $1, get name",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1500.0,
      160.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-207",
     "maxclass": "newobj",
     "text": "live.object",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1500.0,
      200.0,
      80.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-208",
     "maxclass": "newobj",
     "text": "route name",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      ""
     ],
     "patching_rect": [
      1500.0,
      240.0,
      120.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-209",
     "maxclass": "newobj",
     "text": "sscanf %d",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1500.0,
      280.0,
      80.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-210",
     "maxclass": "newobj",
     "text": "prepend /preset/select",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1500.0,
      320.0,
      180.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-211",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      1500.0,
      360.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-300",
     "maxclass": "umenu",
     "patching_rect": [
      1200.0,
      40.0,
      150.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      14.0,
      134.0,
      150.0,
      20.0
     ],
     "items": [
      "MultiPass Test",
      ",",
      "Star Bass Pulse",
      ",",
      "Noise Level Reactive",
      ",",
      "Star Static Blue",
      ",",
      "Video Glitch Chain",
      ",",
      "Video Datamosh",
      ",",
      "Kaleido Mosaic"
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "int",
      "",
      ""
     ],
     "fontsize": 10.0
    }
   },
   {
    "box": {
     "id": "obj-301",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      80.0,
      150.0,
      22.0
     ],
     "text": "prepend /preset/select",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-302",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      120.0,
      60.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-303",
     "maxclass": "newobj",
     "patching_rect": [
      1400.0,
      40.0,
      80.0,
      22.0
     ],
     "text": "prepend set",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-304",
     "maxclass": "comment",
     "patching_rect": [
      1450.0,
      100.0,
      50.0,
      18.0
     ],
     "text": "Param",
     "presentation": 1,
     "presentation_rect": [
      302.0,
      20.0,
      44.0,
      13.0
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-305",
     "maxclass": "comment",
     "patching_rect": [
      1450.0,
      196.0,
      50.0,
      18.0
     ],
     "text": "Stage",
     "presentation": 1,
     "presentation_rect": [
      302.0,
      116.0,
      44.0,
      13.0
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-306",
     "maxclass": "comment",
     "patching_rect": [
      1450.0,
      212.0,
      50.0,
      18.0
     ],
     "text": "Min",
     "presentation": 1,
     "presentation_rect": [
      302.0,
      132.0,
      44.0,
      13.0
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-307",
     "maxclass": "comment",
     "patching_rect": [
      1450.0,
      228.0,
      50.0,
      18.0
     ],
     "text": "Max",
     "presentation": 1,
     "presentation_rect": [
      302.0,
      148.0,
      44.0,
      13.0
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-310",
     "maxclass": "textedit",
     "patching_rect": [
      1200.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      350.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0,
     "text": "amount"
    }
   },
   {
    "box": {
     "id": "obj-314",
     "maxclass": "live.dial",
     "patching_rect": [
      1200.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      364.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_1",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "RGB Shift",
       "parameter_shortname": "RGB Shift",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-311",
     "maxclass": "flonum",
     "patching_rect": [
      1200.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      350.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-312",
     "maxclass": "flonum",
     "patching_rect": [
      1200.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      350.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-313",
     "maxclass": "flonum",
     "patching_rect": [
      1200.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      350.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-315",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 0. 0.05",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-316",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-317",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack 0 amount 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-318",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-319",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-320",
     "maxclass": "textedit",
     "patching_rect": [
      1380.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      430.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0,
     "text": "intensity"
    }
   },
   {
    "box": {
     "id": "obj-324",
     "maxclass": "live.dial",
     "patching_rect": [
      1380.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      444.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_2",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "BG Intensity",
       "parameter_shortname": "BG Intensity",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-321",
     "maxclass": "flonum",
     "patching_rect": [
      1380.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      430.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-322",
     "maxclass": "flonum",
     "patching_rect": [
      1380.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      430.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-323",
     "maxclass": "flonum",
     "patching_rect": [
      1380.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      430.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-325",
     "maxclass": "newobj",
     "patching_rect": [
      1380.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 0. 1.",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-326",
     "maxclass": "newobj",
     "patching_rect": [
      1380.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-327",
     "maxclass": "newobj",
     "patching_rect": [
      1380.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack 1 intensity 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-328",
     "maxclass": "newobj",
     "patching_rect": [
      1380.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-329",
     "maxclass": "newobj",
     "patching_rect": [
      1380.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-330",
     "maxclass": "textedit",
     "patching_rect": [
      1560.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      510.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0,
     "text": "blockCount"
    }
   },
   {
    "box": {
     "id": "obj-334",
     "maxclass": "live.dial",
     "patching_rect": [
      1560.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      524.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_3",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Block Count",
       "parameter_shortname": "Block Count",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-331",
     "maxclass": "flonum",
     "patching_rect": [
      1560.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      510.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-332",
     "maxclass": "flonum",
     "patching_rect": [
      1560.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      510.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-333",
     "maxclass": "flonum",
     "patching_rect": [
      1560.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      510.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-335",
     "maxclass": "newobj",
     "patching_rect": [
      1560.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 4. 64.",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-336",
     "maxclass": "newobj",
     "patching_rect": [
      1560.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-337",
     "maxclass": "newobj",
     "patching_rect": [
      1560.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack 1 blockCount 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-338",
     "maxclass": "newobj",
     "patching_rect": [
      1560.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-339",
     "maxclass": "newobj",
     "patching_rect": [
      1560.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-340",
     "maxclass": "textedit",
     "patching_rect": [
      1740.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      590.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0,
     "text": "jitterAmount"
    }
   },
   {
    "box": {
     "id": "obj-344",
     "maxclass": "live.dial",
     "patching_rect": [
      1740.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      604.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_4",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Jitter Amt",
       "parameter_shortname": "Jitter Amt",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-341",
     "maxclass": "flonum",
     "patching_rect": [
      1740.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      590.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-342",
     "maxclass": "flonum",
     "patching_rect": [
      1740.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      590.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-343",
     "maxclass": "flonum",
     "patching_rect": [
      1740.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      590.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-345",
     "maxclass": "newobj",
     "patching_rect": [
      1740.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 0. 0.1",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-346",
     "maxclass": "newobj",
     "patching_rect": [
      1740.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-347",
     "maxclass": "newobj",
     "patching_rect": [
      1740.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack 2 jitterAmount 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-348",
     "maxclass": "newobj",
     "patching_rect": [
      1740.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-349",
     "maxclass": "newobj",
     "patching_rect": [
      1740.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-350",
     "maxclass": "textedit",
     "patching_rect": [
      1920.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      670.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0,
     "text": "lineDensity"
    }
   },
   {
    "box": {
     "id": "obj-354",
     "maxclass": "live.dial",
     "patching_rect": [
      1920.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      684.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_5",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Line Density",
       "parameter_shortname": "Line Density",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-351",
     "maxclass": "flonum",
     "patching_rect": [
      1920.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      670.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-352",
     "maxclass": "flonum",
     "patching_rect": [
      1920.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      670.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-353",
     "maxclass": "flonum",
     "patching_rect": [
      1920.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      670.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-355",
     "maxclass": "newobj",
     "patching_rect": [
      1920.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 40. 400.",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-356",
     "maxclass": "newobj",
     "patching_rect": [
      1920.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-357",
     "maxclass": "newobj",
     "patching_rect": [
      1920.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack 2 lineDensity 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-358",
     "maxclass": "newobj",
     "patching_rect": [
      1920.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-359",
     "maxclass": "newobj",
     "patching_rect": [
      1920.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-360",
     "maxclass": "textedit",
     "patching_rect": [
      2100.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      750.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-364",
     "maxclass": "live.dial",
     "patching_rect": [
      2100.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      764.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_6",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "FX 6 (free)",
       "parameter_shortname": "FX 6 (free)",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-361",
     "maxclass": "flonum",
     "patching_rect": [
      2100.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      750.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-362",
     "maxclass": "flonum",
     "patching_rect": [
      2100.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      750.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-363",
     "maxclass": "flonum",
     "patching_rect": [
      2100.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      750.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-365",
     "maxclass": "newobj",
     "patching_rect": [
      2100.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 0. 100.",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-366",
     "maxclass": "newobj",
     "patching_rect": [
      2100.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-367",
     "maxclass": "newobj",
     "patching_rect": [
      2100.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack -1 unused 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-368",
     "maxclass": "newobj",
     "patching_rect": [
      2100.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-369",
     "maxclass": "newobj",
     "patching_rect": [
      2100.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-370",
     "maxclass": "textedit",
     "patching_rect": [
      2280.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      830.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-374",
     "maxclass": "live.dial",
     "patching_rect": [
      2280.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      844.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_7",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "FX 7 (free)",
       "parameter_shortname": "FX 7 (free)",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-371",
     "maxclass": "flonum",
     "patching_rect": [
      2280.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      830.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-372",
     "maxclass": "flonum",
     "patching_rect": [
      2280.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      830.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-373",
     "maxclass": "flonum",
     "patching_rect": [
      2280.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      830.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-375",
     "maxclass": "newobj",
     "patching_rect": [
      2280.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 0. 100.",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-376",
     "maxclass": "newobj",
     "patching_rect": [
      2280.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-377",
     "maxclass": "newobj",
     "patching_rect": [
      2280.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack -1 unused 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-378",
     "maxclass": "newobj",
     "patching_rect": [
      2280.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-379",
     "maxclass": "newobj",
     "patching_rect": [
      2280.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-380",
     "maxclass": "textedit",
     "patching_rect": [
      2460.0,
      200.0,
      100.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      910.0,
      20.0,
      72.0,
      16.0
     ],
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "",
      "",
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-384",
     "maxclass": "live.dial",
     "patching_rect": [
      2460.0,
      240.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      924.0,
      36.0,
      44.0,
      48.0
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "varname": "fx_dial_8",
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "FX 8 (free)",
       "parameter_shortname": "FX 8 (free)",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_unitstyle": 0,
       "parameter_modmode": 0
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-381",
     "maxclass": "flonum",
     "patching_rect": [
      2460.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      910.0,
      116.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-382",
     "maxclass": "flonum",
     "patching_rect": [
      2460.0,
      320.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      910.0,
      132.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-383",
     "maxclass": "flonum",
     "patching_rect": [
      2460.0,
      360.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      910.0,
      148.0,
      40.0,
      15.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-385",
     "maxclass": "newobj",
     "patching_rect": [
      2460.0,
      400.0,
      130.0,
      22.0
     ],
     "text": "scale 0. 100. 0. 100.",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-386",
     "maxclass": "newobj",
     "patching_rect": [
      2460.0,
      440.0,
      50.0,
      22.0
     ],
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "obj-387",
     "maxclass": "newobj",
     "patching_rect": [
      2460.0,
      480.0,
      80.0,
      22.0
     ],
     "text": "pack -1 unused 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-388",
     "maxclass": "newobj",
     "patching_rect": [
      2460.0,
      520.0,
      130.0,
      22.0
     ],
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-389",
     "maxclass": "newobj",
     "patching_rect": [
      2460.0,
      560.0,
      50.0,
      22.0
     ],
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-500",
     "maxclass": "comment",
     "patching_rect": [
      1200.0,
      700.0,
      100.0,
      14.0
     ],
     "text": "Show Display",
     "presentation": 1,
     "presentation_rect": [
      236.0,
      60.0,
      100.0,
      13.0
     ],
     "fontsize": 9.0
    }
   },
   {
    "box": {
     "id": "obj-501",
     "maxclass": "button",
     "patching_rect": [
      1200.0,
      720.0,
      22.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      240.0,
      76.0,
      22.0,
      22.0
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ]
    }
   },
   {
    "box": {
     "id": "obj-502",
     "maxclass": "message",
     "patching_rect": [
      1200.0,
      760.0,
      420.0,
      22.0
     ],
     "text": "powershell -ExecutionPolicy Bypass -File \"C:/Users/ROI/Desktop/VJ VST/engine/show_or_launch.ps1\"",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-503",
     "maxclass": "newobj",
     "patching_rect": [
      1200.0,
      800.0,
      100.0,
      22.0
     ],
     "text": "shell",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "obj-504",
     "maxclass": "newobj",
     "text": "prepend set",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      1200.0,
      840.0,
      100.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-505",
     "maxclass": "comment",
     "text": "(not clicked yet)",
     "fontsize": 9.0,
     "presentation": 1,
     "presentation_rect": [
      270.0,
      78.0,
      170.0,
      20.0
     ],
     "patching_rect": [
      1200.0,
      880.0,
      200.0,
      26.0
     ]
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "obj-1",
      0
     ],
     "destination": [
      "obj-2",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-1",
      1
     ],
     "destination": [
      "obj-2",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-1",
      0
     ],
     "destination": [
      "obj-60",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-1",
      1
     ],
     "destination": [
      "obj-60",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-3",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-4",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-6",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-7",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-2",
      0
     ],
     "destination": [
      "obj-92",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-4",
      0
     ],
     "destination": [
      "obj-5",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-3",
      0
     ],
     "destination": [
      "obj-5",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-4",
      0
     ],
     "destination": [
      "obj-6",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-3",
      0
     ],
     "destination": [
      "obj-8",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-3",
      0
     ],
     "destination": [
      "obj-94",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-5",
      0
     ],
     "destination": [
      "obj-9",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-5",
      0
     ],
     "destination": [
      "obj-96",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-10",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-6",
      0
     ],
     "destination": [
      "obj-98",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-7",
      0
     ],
     "destination": [
      "obj-11",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-8",
      0
     ],
     "destination": [
      "obj-12",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-9",
      0
     ],
     "destination": [
      "obj-13",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-10",
      0
     ],
     "destination": [
      "obj-14",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-15",
      0
     ],
     "destination": [
      "obj-16",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-16",
      0
     ],
     "destination": [
      "obj-17",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-46",
      0
     ],
     "destination": [
      "obj-63",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-63",
      0
     ],
     "destination": [
      "obj-15",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-11",
      0
     ],
     "destination": [
      "obj-18",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-17",
      0
     ],
     "destination": [
      "obj-18",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-12",
      0
     ],
     "destination": [
      "obj-19",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-17",
      1
     ],
     "destination": [
      "obj-19",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-13",
      0
     ],
     "destination": [
      "obj-20",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-17",
      2
     ],
     "destination": [
      "obj-20",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-14",
      0
     ],
     "destination": [
      "obj-21",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-17",
      3
     ],
     "destination": [
      "obj-21",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-17",
      4
     ],
     "destination": [
      "obj-73",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-18",
      0
     ],
     "destination": [
      "obj-22",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-22",
      0
     ],
     "destination": [
      "obj-23",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-19",
      0
     ],
     "destination": [
      "obj-24",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-24",
      0
     ],
     "destination": [
      "obj-25",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-20",
      0
     ],
     "destination": [
      "obj-26",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-26",
      0
     ],
     "destination": [
      "obj-27",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-21",
      0
     ],
     "destination": [
      "obj-28",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-28",
      0
     ],
     "destination": [
      "obj-29",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-23",
      0
     ],
     "destination": [
      "obj-30",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-25",
      0
     ],
     "destination": [
      "obj-31",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-27",
      0
     ],
     "destination": [
      "obj-32",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-29",
      0
     ],
     "destination": [
      "obj-33",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-23",
      0
     ],
     "destination": [
      "obj-35",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-25",
      0
     ],
     "destination": [
      "obj-36",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-27",
      0
     ],
     "destination": [
      "obj-37",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-29",
      0
     ],
     "destination": [
      "obj-38",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-30",
      0
     ],
     "destination": [
      "obj-42",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-31",
      0
     ],
     "destination": [
      "obj-43",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-32",
      0
     ],
     "destination": [
      "obj-44",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-33",
      0
     ],
     "destination": [
      "obj-45",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-42",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-42",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-43",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-43",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-44",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-44",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-45",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-45",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-77",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-77",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-40",
      0
     ],
     "destination": [
      "obj-34",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-46",
      0
     ],
     "destination": [
      "obj-47",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-46",
      0
     ],
     "destination": [
      "obj-48",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-46",
      0
     ],
     "destination": [
      "obj-71",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-47",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-48",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-71",
      0
     ],
     "destination": [
      "obj-72",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-73",
      0
     ],
     "destination": [
      "obj-72",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-72",
      0
     ],
     "destination": [
      "obj-74",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-74",
      0
     ],
     "destination": [
      "obj-75",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-75",
      0
     ],
     "destination": [
      "obj-76",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-75",
      0
     ],
     "destination": [
      "obj-78",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-76",
      0
     ],
     "destination": [
      "obj-77",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-111",
      0
     ],
     "destination": [
      "obj-112",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-112",
      0
     ],
     "destination": [
      "obj-113",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-113",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-113",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-114",
      0
     ],
     "destination": [
      "obj-115",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-115",
      0
     ],
     "destination": [
      "obj-116",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-116",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-116",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-117",
      0
     ],
     "destination": [
      "obj-118",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-118",
      0
     ],
     "destination": [
      "obj-119",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-119",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-119",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-25",
      0
     ],
     "destination": [
      "obj-124",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-124",
      0
     ],
     "destination": [
      "obj-125",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-125",
      0
     ],
     "destination": [
      "obj-126",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-126",
      0
     ],
     "destination": [
      "obj-127",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-127",
      0
     ],
     "destination": [
      "obj-128",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-128",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-128",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-46",
      0
     ],
     "destination": [
      "obj-201",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-201",
      0
     ],
     "destination": [
      "obj-202",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-15",
      0
     ],
     "destination": [
      "obj-203",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-203",
      0
     ],
     "destination": [
      "obj-204",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-204",
      0
     ],
     "destination": [
      "obj-202",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-202",
      0
     ],
     "destination": [
      "obj-205",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-205",
      0
     ],
     "destination": [
      "obj-206",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-206",
      0
     ],
     "destination": [
      "obj-207",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-207",
      0
     ],
     "destination": [
      "obj-208",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-208",
      0
     ],
     "destination": [
      "obj-209",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-209",
      0
     ],
     "destination": [
      "obj-210",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-210",
      0
     ],
     "destination": [
      "obj-211",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-211",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-211",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-300",
      0
     ],
     "destination": [
      "obj-301",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-301",
      0
     ],
     "destination": [
      "obj-302",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-302",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-302",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-111",
      0
     ],
     "destination": [
      "obj-303",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-303",
      0
     ],
     "destination": [
      "obj-300",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-314",
      0
     ],
     "destination": [
      "obj-315",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-312",
      0
     ],
     "destination": [
      "obj-315",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-313",
      0
     ],
     "destination": [
      "obj-315",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-315",
      0
     ],
     "destination": [
      "obj-316",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-316",
      1
     ],
     "destination": [
      "obj-317",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-316",
      0
     ],
     "destination": [
      "obj-317",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-311",
      0
     ],
     "destination": [
      "obj-317",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-310",
      0
     ],
     "destination": [
      "obj-317",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-317",
      0
     ],
     "destination": [
      "obj-318",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-318",
      0
     ],
     "destination": [
      "obj-319",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-319",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-319",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-324",
      0
     ],
     "destination": [
      "obj-325",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-322",
      0
     ],
     "destination": [
      "obj-325",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-323",
      0
     ],
     "destination": [
      "obj-325",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-325",
      0
     ],
     "destination": [
      "obj-326",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-326",
      1
     ],
     "destination": [
      "obj-327",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-326",
      0
     ],
     "destination": [
      "obj-327",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-321",
      0
     ],
     "destination": [
      "obj-327",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-320",
      0
     ],
     "destination": [
      "obj-327",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-327",
      0
     ],
     "destination": [
      "obj-328",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-328",
      0
     ],
     "destination": [
      "obj-329",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-329",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-329",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-334",
      0
     ],
     "destination": [
      "obj-335",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-332",
      0
     ],
     "destination": [
      "obj-335",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-333",
      0
     ],
     "destination": [
      "obj-335",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-335",
      0
     ],
     "destination": [
      "obj-336",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-336",
      1
     ],
     "destination": [
      "obj-337",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-336",
      0
     ],
     "destination": [
      "obj-337",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-331",
      0
     ],
     "destination": [
      "obj-337",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-330",
      0
     ],
     "destination": [
      "obj-337",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-337",
      0
     ],
     "destination": [
      "obj-338",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-338",
      0
     ],
     "destination": [
      "obj-339",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-339",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-339",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-344",
      0
     ],
     "destination": [
      "obj-345",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-342",
      0
     ],
     "destination": [
      "obj-345",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-343",
      0
     ],
     "destination": [
      "obj-345",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-345",
      0
     ],
     "destination": [
      "obj-346",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-346",
      1
     ],
     "destination": [
      "obj-347",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-346",
      0
     ],
     "destination": [
      "obj-347",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-341",
      0
     ],
     "destination": [
      "obj-347",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-340",
      0
     ],
     "destination": [
      "obj-347",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-347",
      0
     ],
     "destination": [
      "obj-348",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-348",
      0
     ],
     "destination": [
      "obj-349",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-349",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-349",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-354",
      0
     ],
     "destination": [
      "obj-355",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-352",
      0
     ],
     "destination": [
      "obj-355",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-353",
      0
     ],
     "destination": [
      "obj-355",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-355",
      0
     ],
     "destination": [
      "obj-356",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-356",
      1
     ],
     "destination": [
      "obj-357",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-356",
      0
     ],
     "destination": [
      "obj-357",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-351",
      0
     ],
     "destination": [
      "obj-357",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-350",
      0
     ],
     "destination": [
      "obj-357",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-357",
      0
     ],
     "destination": [
      "obj-358",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-358",
      0
     ],
     "destination": [
      "obj-359",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-359",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-359",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-364",
      0
     ],
     "destination": [
      "obj-365",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-362",
      0
     ],
     "destination": [
      "obj-365",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-363",
      0
     ],
     "destination": [
      "obj-365",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-365",
      0
     ],
     "destination": [
      "obj-366",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-366",
      1
     ],
     "destination": [
      "obj-367",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-366",
      0
     ],
     "destination": [
      "obj-367",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-361",
      0
     ],
     "destination": [
      "obj-367",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-360",
      0
     ],
     "destination": [
      "obj-367",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-367",
      0
     ],
     "destination": [
      "obj-368",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-368",
      0
     ],
     "destination": [
      "obj-369",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-369",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-369",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-374",
      0
     ],
     "destination": [
      "obj-375",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-372",
      0
     ],
     "destination": [
      "obj-375",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-373",
      0
     ],
     "destination": [
      "obj-375",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-375",
      0
     ],
     "destination": [
      "obj-376",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-376",
      1
     ],
     "destination": [
      "obj-377",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-376",
      0
     ],
     "destination": [
      "obj-377",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-371",
      0
     ],
     "destination": [
      "obj-377",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-370",
      0
     ],
     "destination": [
      "obj-377",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-377",
      0
     ],
     "destination": [
      "obj-378",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-378",
      0
     ],
     "destination": [
      "obj-379",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-379",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-379",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-384",
      0
     ],
     "destination": [
      "obj-385",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-382",
      0
     ],
     "destination": [
      "obj-385",
      3
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-383",
      0
     ],
     "destination": [
      "obj-385",
      4
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-385",
      0
     ],
     "destination": [
      "obj-386",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-386",
      1
     ],
     "destination": [
      "obj-387",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-386",
      0
     ],
     "destination": [
      "obj-387",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-381",
      0
     ],
     "destination": [
      "obj-387",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-380",
      0
     ],
     "destination": [
      "obj-387",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-387",
      0
     ],
     "destination": [
      "obj-388",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-388",
      0
     ],
     "destination": [
      "obj-389",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-389",
      0
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-389",
      1
     ],
     "destination": [
      "obj-40",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-501",
      0
     ],
     "destination": [
      "obj-502",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-502",
      0
     ],
     "destination": [
      "obj-503",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-503",
      0
     ],
     "destination": [
      "obj-504",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-504",
      0
     ],
     "destination": [
      "obj-505",
      0
     ]
    }
   }
  ],
  "parameters": {
   "parameterbanks": {},
   "inherited_shortname": 1
  },
  "dependency_cache": [],
  "autosave": 0
 }
}