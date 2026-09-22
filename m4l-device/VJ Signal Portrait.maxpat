{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 6,
   "revision": 4,
   "architecture": "x64",
   "modernui": 1
  },
  "classnamespace": "dsp.dspchainobject",
  "rect": [
   0,
   44,
   1060,
   560
  ],
  "openinpresentation": 1,
  "default_fontname": "Arial",
  "default_fontsize": 12.0,
  "gridonopen": 1,
  "gridsize": [
   15.0,
   15.0
  ],
  "boxes": [
   {
    "box": {
     "id": "obj-1",
     "maxclass": "newobj",
     "text": "OpenSoundControl",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      880.0,
      340.0,
      130.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-2",
     "maxclass": "newobj",
     "text": "udpsend 127.0.0.1 9000",
     "numinlets": 1,
     "numoutlets": 0,
     "outlettype": [],
     "patching_rect": [
      880.0,
      370.0,
      200.0,
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
      880.0,
      450.0,
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
     "outlettype": [],
     "patching_rect": [
      880.0,
      490.0,
      60.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-60",
     "maxclass": "comment",
     "text": "VJ SIGNAL PORTRAIT",
     "fontsize": 13.0,
     "presentation": 1,
     "presentation_rect": [
      8.0,
      3.0,
      240.0,
      18.0
     ],
     "patching_rect": [
      8.0,
      5.0,
      240.0,
      18.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-61",
     "maxclass": "comment",
     "text": "Preset 08 Signal Portrait. Click Activate to select this preset AND load its reference clip (Media/revenant_portrait.mp4) - without a loaded video the effect chain has nothing to draw (black placeholder), which is why the effects look inert on their own. Wire=edge skeleton (EdgeEtch, stage 0, threshold). Dark=field crush (stage 0, darkness). Chroma=RGB smear (ChromaSmear, stage 1, amount). Drop=signal-loss bars (SignalDrop, stage 2, intensity). All MIDI-mappable. Datamosh (stage 3) is audio-reactive only (onset drives smear, no knob needed).",
     "patching_rect": [
      10.0,
      510.0,
      860.0,
      40.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-62",
     "maxclass": "comment",
     "text": "Activate ->",
     "presentation": 1,
     "presentation_rect": [
      8.0,
      98.0,
      60.0,
      14.0
     ],
     "patching_rect": [
      8.0,
      30.0,
      60.0,
      14.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-10",
     "maxclass": "live.dial",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "patching_rect": [
      10.0,
      110.0,
      44.0,
      52.0
     ],
     "presentation": 1,
     "presentation_rect": [
      10.0,
      22.0,
      44.0,
      52.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Wire Amount",
       "parameter_shortname": "Wire",
       "parameter_mmin": 0.0,
       "parameter_mmax": 1.0,
       "parameter_type": 0,
       "parameter_enable": 1
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-11",
     "maxclass": "comment",
     "text": "Wire",
     "presentation": 1,
     "presentation_rect": [
      10.0,
      76.0,
      44.0,
      16.0
     ],
     "patching_rect": [
      10.0,
      165.0,
      44.0,
      16.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-12",
     "maxclass": "newobj",
     "text": "scale 0. 1. 0.4 0.04",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      200.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-13",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      10.0,
      230.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-14",
     "maxclass": "newobj",
     "text": "pack 0 threshold 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      265.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-15",
     "maxclass": "newobj",
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      295.0,
      185.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-16",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      10.0,
      325.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-20",
     "maxclass": "live.dial",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "patching_rect": [
      230.0,
      110.0,
      44.0,
      52.0
     ],
     "presentation": 1,
     "presentation_rect": [
      70.0,
      22.0,
      44.0,
      52.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Field Darkness",
       "parameter_shortname": "Dark",
       "parameter_mmin": 0.0,
       "parameter_mmax": 1.0,
       "parameter_type": 0,
       "parameter_enable": 1
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-21",
     "maxclass": "comment",
     "text": "Dark",
     "presentation": 1,
     "presentation_rect": [
      70.0,
      76.0,
      44.0,
      16.0
     ],
     "patching_rect": [
      230.0,
      165.0,
      44.0,
      16.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-22",
     "maxclass": "newobj",
     "text": "scale 0. 1. 0.3 0.92",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      230.0,
      200.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-23",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      230.0,
      230.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-24",
     "maxclass": "newobj",
     "text": "pack 0 darkness 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      230.0,
      265.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-25",
     "maxclass": "newobj",
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      230.0,
      295.0,
      185.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-26",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      230.0,
      325.0,
      50.0,
      22.0
     ]
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
      450.0,
      110.0,
      44.0,
      52.0
     ],
     "presentation": 1,
     "presentation_rect": [
      130.0,
      22.0,
      44.0,
      52.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Chroma Smear",
       "parameter_shortname": "Chroma",
       "parameter_mmin": 0.0,
       "parameter_mmax": 1.0,
       "parameter_type": 0,
       "parameter_enable": 1
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-31",
     "maxclass": "comment",
     "text": "Chroma",
     "presentation": 1,
     "presentation_rect": [
      130.0,
      76.0,
      44.0,
      16.0
     ],
     "patching_rect": [
      450.0,
      165.0,
      50.0,
      16.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-32",
     "maxclass": "newobj",
     "text": "scale 0. 1. 0. 0.04",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      450.0,
      200.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-33",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      450.0,
      230.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-34",
     "maxclass": "newobj",
     "text": "pack 1 amount 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      450.0,
      265.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-35",
     "maxclass": "newobj",
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      450.0,
      295.0,
      185.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-36",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      450.0,
      325.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-40",
     "maxclass": "live.dial",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "parameter_enable": 1,
     "patching_rect": [
      670.0,
      110.0,
      44.0,
      52.0
     ],
     "presentation": 1,
     "presentation_rect": [
      190.0,
      22.0,
      44.0,
      52.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Signal Drop",
       "parameter_shortname": "Drop",
       "parameter_mmin": 0.0,
       "parameter_mmax": 1.0,
       "parameter_type": 0,
       "parameter_enable": 1
      }
     }
    }
   },
   {
    "box": {
     "id": "obj-41",
     "maxclass": "comment",
     "text": "Drop",
     "presentation": 1,
     "presentation_rect": [
      190.0,
      76.0,
      44.0,
      16.0
     ],
     "patching_rect": [
      670.0,
      165.0,
      44.0,
      16.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-42",
     "maxclass": "newobj",
     "text": "scale 0. 1. 0. 0.9",
     "numinlets": 5,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      670.0,
      200.0,
      150.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-43",
     "maxclass": "newobj",
     "text": "t b f",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      "float"
     ],
     "patching_rect": [
      670.0,
      230.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-44",
     "maxclass": "newobj",
     "text": "pack 2 intensity 0.",
     "numinlets": 3,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      670.0,
      265.0,
      160.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-45",
     "maxclass": "newobj",
     "text": "prepend /effect/param",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      670.0,
      295.0,
      185.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-46",
     "maxclass": "newobj",
     "text": "t b l",
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "bang",
      ""
     ],
     "patching_rect": [
      670.0,
      325.0,
      50.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-63",
     "maxclass": "button",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      "bang"
     ],
     "patching_rect": [
      10.0,
      400.0,
      20.0,
      20.0
     ],
     "presentation": 1,
     "presentation_rect": [
      70.0,
      96.0,
      18.0,
      18.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-64",
     "maxclass": "message",
     "text": "/preset/select 8",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      430.0,
      140.0,
      22.0
     ]
    }
   },
   {
    "box": {
     "id": "obj-65",
     "maxclass": "message",
     "text": "/video/load \"C:/Users/ROI/Desktop/VJ VST/engine/Media/revenant_portrait.mp4\"",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "patching_rect": [
      10.0,
      460.0,
      500.0,
      22.0
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
   },
   {
    "patchline": {
     "source": [
      "obj-10",
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
      "obj-12",
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
      "obj-13",
      1
     ],
     "destination": [
      "obj-14",
      2
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
      "obj-14",
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
      "obj-15",
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
      1
     ],
     "destination": [
      "obj-1",
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
      "obj-1",
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
      "obj-23",
      1
     ],
     "destination": [
      "obj-24",
      2
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
      "obj-25",
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
      1
     ],
     "destination": [
      "obj-1",
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
      "obj-1",
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
      "obj-32",
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
      "obj-33",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-33",
      1
     ],
     "destination": [
      "obj-34",
      2
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
      "obj-34",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-34",
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
      "obj-35",
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
      "obj-36",
      1
     ],
     "destination": [
      "obj-1",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-36",
      0
     ],
     "destination": [
      "obj-1",
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
      "obj-42",
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
      "obj-43",
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
      "obj-44",
      2
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
      "obj-44",
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
      "obj-45",
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
      "obj-46",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-46",
      1
     ],
     "destination": [
      "obj-1",
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
      "obj-1",
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
      "obj-64",
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
      "obj-65",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-64",
      0
     ],
     "destination": [
      "obj-1",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "obj-65",
      0
     ],
     "destination": [
      "obj-1",
      0
     ]
    }
   }
  ]
 }
}