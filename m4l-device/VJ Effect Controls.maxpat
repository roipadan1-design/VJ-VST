{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 6,
   "revision": 5,
   "architecture": "x64",
   "modernui": 1
  },
  "classnamespace": "box",
  "rect": [
   134.0,
   134.0,
   340.0,
   380.0
  ],
  "openrect": [
   0.0,
   0.0,
   340.0,
   380.0
  ],
  "bglocked": 0,
  "openinpresentation": 1,
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
  "lefttoolbarpinned": 0,
  "toptoolbarpinned": 0,
  "righttoolbarpinned": 0,
  "bottomtoolbarpinned": 0,
  "toolbars_unpinned_last_save": 0,
  "tallnewobj": 0,
  "boxanimatetime": 200,
  "enablehscroll": 1,
  "enablevscroll": 1,
  "devicewidth": 0.0,
  "description": "",
  "digest": "",
  "tags": "",
  "style": "",
  "subpatcher_template": "",
  "showontab": 0,
  "assistshowspatchername": 0,
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "",
      "",
      "OSCTimeTag"
     ],
     "patching_rect": [
      900.0,
      20.0,
      130.0,
      22.0
     ],
     "text": "OpenSoundControl"
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      900.0,
      60.0,
      200.0,
      22.0
     ],
     "text": "udpsend 127.0.0.1 9000"
    }
   },
   {
    "box": {
     "id": "obj-3",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      0.0,
      400.0,
      20.0
     ],
     "text": "VJ EFFECT CONTROLS"
    }
   },
   {
    "box": {
     "id": "obj-4",
     "linecount": 4,
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      720.0,
      860.0,
      62.0
     ],
     "text": "Each row is one generic knob: Stage = effectChain stage index (0-based) of the CURRENT preset; Param = the ISF input name to control (see the preset's shader, e.g. \"blockCount\", \"jitterAmount\"); Min/Max = the real-world range the knob (0-1) maps to. Turning the knob sends /effect/param <stage> <param> <value> to the VJ Engine. Engine side confirmed working via direct OSC. Max-side dial-turn itself not yet manually verified in Live/Max (the earlier Max trial/Demo-license block that prevented new objects from running has since been resolved - see project notes)."
    }
   },
   {
    "box": {
     "id": "obj-5",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      120.0,
      60.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      20.0,
      60.0,
      20.0
     ],
     "text": "Knob 1"
    }
   },
   {
    "box": {
     "id": "obj-6",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      90.0,
      120.0,
      40.0,
      20.0
     ],
     "text": "Stage"
    }
   },
   {
    "box": {
     "id": "obj-7",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      90.0,
      140.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      38.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-8",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      150.0,
      120.0,
      77.0,
      20.0
     ],
     "text": "Param name"
    }
   },
   {
    "box": {
     "id": "obj-9",
     "maxclass": "textedit",
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "int",
      "",
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      150.0,
      140.0,
      120.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      60.0,
      38.0,
      100.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      280.0,
      120.0,
      30.0,
      20.0
     ],
     "text": "Min"
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      280.0,
      140.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      165.0,
      38.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      340.0,
      120.0,
      31.0,
      20.0
     ],
     "text": "Max"
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      340.0,
      140.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      215.0,
      38.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      410.0,
      120.0,
      40.0,
      20.0
     ],
     "text": "Value"
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "live.dial",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "patching_rect": [
      410.0,
      140.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      270.0,
      30.0,
      40.0,
      48.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Effect Knob 1",
       "parameter_mmax": 1.0,
       "parameter_modmode": 0,
       "parameter_shortname": "Knob 1",
       "parameter_type": 0,
       "parameter_unitstyle": 0
      }
     },
     "varname": "live.dial"
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "newobj",
     "numinlets": 6,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      190.0,
      150.0,
      22.0
     ],
     "text": "scale 0. 1. 0. 1."
    }
   },
   {
    "box": {
     "id": "obj-17",
     "maxclass": "newobj",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      220.0,
      150.0,
      22.0
     ],
     "text": "pack 0 s 0."
    }
   },
   {
    "box": {
     "id": "obj-18",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      250.0,
      200.0,
      22.0
     ],
     "text": "prepend /effect/param"
    }
   },
   {
    "box": {
     "id": "obj-19",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      470.0,
      280.0,
      60.0,
      22.0
     ],
     "text": "t b l"
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      260.0,
      60.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      110.0,
      60.0,
      20.0
     ],
     "text": "Knob 2"
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      90.0,
      260.0,
      40.0,
      20.0
     ],
     "text": "Stage"
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      90.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      128.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      150.0,
      260.0,
      77.0,
      20.0
     ],
     "text": "Param name"
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "textedit",
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "int",
      "",
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      150.0,
      280.0,
      120.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      60.0,
      128.0,
      100.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      280.0,
      260.0,
      30.0,
      20.0
     ],
     "text": "Min"
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      280.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      165.0,
      128.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-27",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      340.0,
      260.0,
      31.0,
      20.0
     ],
     "text": "Max"
    }
   },
   {
    "box": {
     "id": "obj-28",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      340.0,
      280.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      215.0,
      128.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-29",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      410.0,
      260.0,
      40.0,
      20.0
     ],
     "text": "Value"
    }
   },
   {
    "box": {
     "id": "obj-30",
     "maxclass": "live.dial",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "patching_rect": [
      410.0,
      280.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      270.0,
      120.0,
      40.0,
      48.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Effect Knob 2",
       "parameter_mmax": 1.0,
       "parameter_modmode": 0,
       "parameter_shortname": "Knob 2",
       "parameter_type": 0,
       "parameter_unitstyle": 0
      }
     },
     "varname": "live.dial[1]"
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "newobj",
     "numinlets": 6,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      330.0,
      150.0,
      22.0
     ],
     "text": "scale 0. 1. 0. 1."
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "newobj",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      360.0,
      150.0,
      22.0
     ],
     "text": "pack 0 s 0."
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      390.0,
      200.0,
      22.0
     ],
     "text": "prepend /effect/param"
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      470.0,
      420.0,
      60.0,
      22.0
     ],
     "text": "t b l"
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      400.0,
      60.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      200.0,
      60.0,
      20.0
     ],
     "text": "Knob 3"
    }
   },
   {
    "box": {
     "id": "obj-36",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      90.0,
      400.0,
      40.0,
      20.0
     ],
     "text": "Stage"
    }
   },
   {
    "box": {
     "id": "obj-37",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      90.0,
      420.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      218.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-38",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      150.0,
      400.0,
      77.0,
      20.0
     ],
     "text": "Param name"
    }
   },
   {
    "box": {
     "id": "obj-39",
     "maxclass": "textedit",
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "int",
      "",
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      150.0,
      420.0,
      120.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      60.0,
      218.0,
      100.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-40",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      280.0,
      400.0,
      30.0,
      20.0
     ],
     "text": "Min"
    }
   },
   {
    "box": {
     "id": "obj-41",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      280.0,
      420.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      165.0,
      218.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-42",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      340.0,
      400.0,
      31.0,
      20.0
     ],
     "text": "Max"
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      340.0,
      420.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      215.0,
      218.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      410.0,
      400.0,
      40.0,
      20.0
     ],
     "text": "Value"
    }
   },
   {
    "box": {
     "id": "obj-45",
     "maxclass": "live.dial",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "patching_rect": [
      410.0,
      420.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      270.0,
      210.0,
      40.0,
      48.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Effect Knob 3",
       "parameter_mmax": 1.0,
       "parameter_modmode": 0,
       "parameter_shortname": "Knob 3",
       "parameter_type": 0,
       "parameter_unitstyle": 0
      }
     },
     "varname": "live.dial[2]"
    }
   },
   {
    "box": {
     "id": "obj-46",
     "maxclass": "newobj",
     "numinlets": 6,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      470.0,
      150.0,
      22.0
     ],
     "text": "scale 0. 1. 0. 1."
    }
   },
   {
    "box": {
     "id": "obj-47",
     "maxclass": "newobj",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      500.0,
      150.0,
      22.0
     ],
     "text": "pack 0 s 0."
    }
   },
   {
    "box": {
     "id": "obj-48",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      530.0,
      200.0,
      22.0
     ],
     "text": "prepend /effect/param"
    }
   },
   {
    "box": {
     "id": "obj-49",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      470.0,
      560.0,
      60.0,
      22.0
     ],
     "text": "t b l"
    }
   },
   {
    "box": {
     "id": "obj-50",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      20.0,
      540.0,
      60.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      290.0,
      60.0,
      20.0
     ],
     "text": "Knob 4"
    }
   },
   {
    "box": {
     "id": "obj-51",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      90.0,
      540.0,
      40.0,
      20.0
     ],
     "text": "Stage"
    }
   },
   {
    "box": {
     "id": "obj-52",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      90.0,
      560.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      308.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-53",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      150.0,
      540.0,
      77.0,
      20.0
     ],
     "text": "Param name"
    }
   },
   {
    "box": {
     "id": "obj-54",
     "maxclass": "textedit",
     "numinlets": 1,
     "numoutlets": 4,
     "outlettype": [
      "",
      "int",
      "",
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      150.0,
      560.0,
      120.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      60.0,
      308.0,
      100.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-55",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      280.0,
      540.0,
      30.0,
      20.0
     ],
     "text": "Min"
    }
   },
   {
    "box": {
     "id": "obj-56",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      280.0,
      560.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      165.0,
      308.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-57",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      340.0,
      540.0,
      31.0,
      20.0
     ],
     "text": "Max"
    }
   },
   {
    "box": {
     "id": "obj-58",
     "maxclass": "flonum",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "parameter_enable": 0,
     "patching_rect": [
      340.0,
      560.0,
      50.0,
      22.0
     ],
     "presentation": 1,
     "presentation_rect": [
      215.0,
      308.0,
      45.0,
      20.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-59",
     "maxclass": "comment",
     "numinlets": 1,
     "numoutlets": 0,
     "patching_rect": [
      410.0,
      540.0,
      40.0,
      20.0
     ],
     "text": "Value"
    }
   },
   {
    "box": {
     "id": "obj-60",
     "maxclass": "live.dial",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "patching_rect": [
      410.0,
      560.0,
      44.0,
      48.0
     ],
     "presentation": 1,
     "presentation_rect": [
      270.0,
      300.0,
      40.0,
      48.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Effect Knob 4",
       "parameter_mmax": 1.0,
       "parameter_modmode": 0,
       "parameter_shortname": "Knob 4",
       "parameter_type": 0,
       "parameter_unitstyle": 0
      }
     },
     "varname": "live.dial[3]"
    }
   },
   {
    "box": {
     "id": "obj-61",
     "maxclass": "newobj",
     "numinlets": 6,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      610.0,
      150.0,
      22.0
     ],
     "text": "scale 0. 1. 0. 1."
    }
   },
   {
    "box": {
     "id": "obj-62",
     "maxclass": "newobj",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      640.0,
      150.0,
      22.0
     ],
     "text": "pack 0 s 0."
    }
   },
   {
    "box": {
     "id": "obj-63",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      470.0,
      670.0,
      200.0,
      22.0
     ],
     "text": "prepend /effect/param"
    }
   },
   {
    "box": {
     "id": "obj-64",
     "maxclass": "newobj",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      470.0,
      700.0,
      60.0,
      22.0
     ],
     "text": "t b l"
    }
   },
   {
    "box": {
     "id": "obj-100",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      500.0,
      200.0,
      40.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-101",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      500.0,
      200.0,
      40.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-102",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      500.0,
      200.0,
      40.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-103",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      500.0,
      200.0,
      40.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-200",
     "maxclass": "newobj",
     "text": "plugin~",
     "numinlets": 0,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      "signal"
     ],
     "patching_rect": [
      700.0,
      20.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-201",
     "maxclass": "newobj",
     "text": "plugout~",
     "numinlets": 2,
     "numoutlets": 0,
     "patching_rect": [
      700.0,
      60.0,
      60.0,
      22.0
     ]
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "destination": [
      "obj-2",
      0
     ],
     "source": [
      "obj-1",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-16",
      2
     ],
     "source": [
      "obj-11",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-16",
      3
     ],
     "source": [
      "obj-13",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-16",
      0
     ],
     "source": [
      "obj-15",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-18",
      0
     ],
     "source": [
      "obj-17",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-19",
      0
     ],
     "source": [
      "obj-18",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-19",
      1
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-19",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-32",
      0
     ],
     "source": [
      "obj-22",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-32",
      1
     ],
     "source": [
      "obj-24",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-31",
      2
     ],
     "source": [
      "obj-26",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-31",
      3
     ],
     "source": [
      "obj-28",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-31",
      0
     ],
     "source": [
      "obj-30",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-33",
      0
     ],
     "source": [
      "obj-32",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-34",
      0
     ],
     "source": [
      "obj-33",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-34",
      1
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-34",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-47",
      0
     ],
     "source": [
      "obj-37",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-47",
      1
     ],
     "source": [
      "obj-39",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-46",
      2
     ],
     "source": [
      "obj-41",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-46",
      3
     ],
     "source": [
      "obj-43",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-46",
      0
     ],
     "source": [
      "obj-45",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-48",
      0
     ],
     "source": [
      "obj-47",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-49",
      0
     ],
     "source": [
      "obj-48",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-49",
      1
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-49",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-62",
      0
     ],
     "source": [
      "obj-52",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-62",
      1
     ],
     "source": [
      "obj-54",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-61",
      2
     ],
     "source": [
      "obj-56",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-61",
      3
     ],
     "source": [
      "obj-58",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-61",
      0
     ],
     "source": [
      "obj-60",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-63",
      0
     ],
     "source": [
      "obj-62",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-64",
      0
     ],
     "source": [
      "obj-63",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-64",
      1
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-1",
      0
     ],
     "source": [
      "obj-64",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-17",
      0
     ],
     "source": [
      "obj-7",
      0
     ]
    }
   },
   {
    "patchline": {
     "destination": [
      "obj-17",
      1
     ],
     "source": [
      "obj-9",
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
      "obj-100",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-100",
      1
     ],
     "destination": [
      "obj-17",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-100",
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
      "obj-31",
      0
     ],
     "destination": [
      "obj-101",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-101",
      1
     ],
     "destination": [
      "obj-32",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-101",
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
      "obj-46",
      0
     ],
     "destination": [
      "obj-102",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-102",
      1
     ],
     "destination": [
      "obj-47",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-102",
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
      "obj-61",
      0
     ],
     "destination": [
      "obj-103",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-103",
      1
     ],
     "destination": [
      "obj-62",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-103",
      0
     ],
     "destination": [
      "obj-62",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-200",
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
      "obj-200",
      1
     ],
     "destination": [
      "obj-201",
      1
     ]
    }
   }
  ],
  "parameters": {
   "obj-15": [
    "Effect Knob 1",
    "Knob 1",
    0
   ],
   "obj-30": [
    "Effect Knob 2",
    "Knob 2",
    0
   ],
   "obj-45": [
    "Effect Knob 3",
    "Knob 3",
    0
   ],
   "obj-60": [
    "Effect Knob 4",
    "Knob 4",
    0
   ],
   "parameterbanks": {
    "0": {
     "index": 0,
     "name": "",
     "parameters": [
      "-",
      "-",
      "-",
      "-",
      "-",
      "-",
      "-",
      "-"
     ]
    }
   },
   "inherited_shortname": 1
  },
  "dependency_cache": [
   {
    "name": "OpenSoundControl.mxe64",
    "type": "mx64"
   }
  ],
  "autosave": 0
 }
}