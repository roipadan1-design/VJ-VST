{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 8,
      "minor": 5,
      "revision": 5,
      "architecture": "x64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      80,
      80,
      1100,
      1000
    ],
    "openrect": [
      0,
      0,
      370,
      200
    ],
    "bglocked": 0,
    "openinpresentation": 1,
    "default_fontsize": 12,
    "default_fontface": 0,
    "default_fontname": "Arial",
    "gridonopen": 1,
    "gridsize": [
      15,
      15
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
    "devicewidth": 0,
    "description": "",
    "digest": "",
    "tags": "",
    "style": "",
    "subpatcher_template": "",
    "assistshowspatchername": 0,
    "title": "Max Audio Effect",
    "boxes": [
      {
        "box": {
          "id": "obj-1",
          "maxclass": "comment",
          "text": "SPECTRO TUNNEL v2 — audio analysis + transport + transients -> Node -> WebSocket -> Chrome",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30,
            8,
            560,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-2",
          "maxclass": "newobj",
          "text": "plugin~",
          "numinlets": 0,
          "numoutlets": 2,
          "patching_rect": [
            40,
            50,
            60,
            22
          ],
          "outlettype": [
            "signal",
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-3",
          "maxclass": "newobj",
          "text": "plugout~",
          "numinlets": 2,
          "numoutlets": 0,
          "patching_rect": [
            40,
            620,
            66,
            22
          ]
        }
      },
      {
        "box": {
          "id": "obj-4",
          "maxclass": "newobj",
          "text": "+~",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            200,
            90,
            40,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-5",
          "maxclass": "newobj",
          "text": "*~ 0.5",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            200,
            125,
            50,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-6",
          "maxclass": "newobj",
          "text": "reson~ 1. 60 5",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            40,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-7",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-8",
          "maxclass": "newobj",
          "text": "reson~ 1. 150 6",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            135,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-9",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            135,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-10",
          "maxclass": "newobj",
          "text": "reson~ 1. 380 7",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            230,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-11",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            230,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-12",
          "maxclass": "newobj",
          "text": "reson~ 1. 850 8",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            325,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-13",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            325,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-14",
          "maxclass": "newobj",
          "text": "reson~ 1. 1800 9",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            420,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-15",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            420,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-16",
          "maxclass": "newobj",
          "text": "reson~ 1. 3600 10",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            515,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-17",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            515,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-18",
          "maxclass": "newobj",
          "text": "reson~ 1. 7000 11",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            610,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-19",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            610,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-20",
          "maxclass": "newobj",
          "text": "reson~ 1. 12000 12",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            705,
            200,
            90,
            22
          ],
          "outlettype": [
            "signal"
          ]
        }
      },
      {
        "box": {
          "id": "obj-21",
          "maxclass": "newobj",
          "text": "avg~",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            705,
            245,
            50,
            22
          ],
          "outlettype": [
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-22",
          "maxclass": "newobj",
          "text": "loadbang",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            700,
            50,
            70,
            22
          ],
          "outlettype": [
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-23",
          "maxclass": "message",
          "text": "1",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            700,
            85,
            24,
            22
          ]
        }
      },
      {
        "box": {
          "id": "obj-24",
          "maxclass": "newobj",
          "text": "metro 16",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            700,
            120,
            70,
            22
          ],
          "outlettype": [
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-25",
          "maxclass": "newobj",
          "text": "t b b b b b b b b",
          "numinlets": 1,
          "numoutlets": 8,
          "patching_rect": [
            700,
            160,
            200,
            22
          ],
          "outlettype": [
            "bang",
            "bang",
            "bang",
            "bang",
            "bang",
            "bang",
            "bang",
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-26",
          "maxclass": "newobj",
          "text": "pack 0. 0. 0. 0. 0. 0. 0. 0.",
          "numinlets": 8,
          "numoutlets": 1,
          "patching_rect": [
            700,
            360,
            200,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-27",
          "maxclass": "newobj",
          "text": "prepend spec8",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            700,
            400,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-28",
          "maxclass": "newobj",
          "text": "node.script bridge.js @autostart 1 @watch 1",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            700,
            460,
            280,
            22
          ],
          "outlettype": [
            "",
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-29",
          "maxclass": "comment",
          "text": "bridge.js: serves engine/ on :8765, broadcasts analysis + triggers over WS",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            700,
            500,
            380,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-30",
          "maxclass": "comment",
          "text": "TRANSPORT — beat/bar/bpm from Live",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            40,
            650,
            260,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-31",
          "maxclass": "newobj",
          "text": "plugsync~",
          "numinlets": 0,
          "numoutlets": 5,
          "patching_rect": [
            40,
            690,
            90,
            22
          ],
          "outlettype": [
            "signal",
            "signal",
            "signal",
            "float",
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-32",
          "maxclass": "newobj",
          "text": "floor",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            730,
            50,
            22
          ],
          "outlettype": [
            "int"
          ]
        }
      },
      {
        "box": {
          "id": "obj-33",
          "maxclass": "newobj",
          "text": "change",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            760,
            55,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-34",
          "maxclass": "newobj",
          "text": "floor",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            160,
            730,
            50,
            22
          ],
          "outlettype": [
            "int"
          ]
        }
      },
      {
        "box": {
          "id": "obj-35",
          "maxclass": "newobj",
          "text": "change",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            160,
            760,
            55,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-36",
          "maxclass": "newobj",
          "text": "transport",
          "numinlets": 1,
          "numoutlets": 5,
          "patching_rect": [
            300,
            690,
            80,
            22
          ],
          "outlettype": [
            "int",
            "",
            "float",
            "float",
            "float"
          ]
        }
      },
      {
        "box": {
          "id": "obj-37",
          "maxclass": "newobj",
          "text": "pack 1 1 120.",
          "numinlets": 3,
          "numoutlets": 1,
          "patching_rect": [
            40,
            800,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-38",
          "maxclass": "newobj",
          "text": "prepend transport",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            835,
            130,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-39",
          "maxclass": "comment",
          "text": "FFT ANALYSIS — spectral flux for transient detection",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            400,
            650,
            320,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-40",
          "maxclass": "newobj",
          "text": "pfft~ spectro_flux 1024 4",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            "signal"
          ],
          "patching_rect": [
            400,
            690,
            180,
            22
          ],
          "patcher": {
            "fileversion": 1,
            "appversion": {
              "major": 8,
              "minor": 5,
              "revision": 5,
              "architecture": "x64",
              "modernui": 1
            },
            "classnamespace": "box",
            "rect": [
              60,
              80,
              300,
              200
            ],
            "boxes": [
              {
                "box": {
                  "id": "obj-41",
                  "maxclass": "newobj",
                  "text": "fftin~ 1",
                  "numinlets": 0,
                  "numoutlets": 3,
                  "outlettype": [
                    "signal",
                    "signal",
                    "signal"
                  ],
                  "patching_rect": [
                    40,
                    30,
                    70,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-42",
                  "maxclass": "newobj",
                  "text": "cartopol~",
                  "numinlets": 2,
                  "numoutlets": 2,
                  "outlettype": [
                    "signal",
                    "signal"
                  ],
                  "patching_rect": [
                    40,
                    70,
                    75,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-43",
                  "maxclass": "newobj",
                  "text": "fftout~ 1",
                  "numinlets": 1,
                  "numoutlets": 0,
                  "patching_rect": [
                    40,
                    110,
                    70,
                    22
                  ]
                }
              }
            ],
            "lines": [
              {
                "patchline": {
                  "source": [
                    "obj-41",
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
                    "obj-41",
                    1
                  ],
                  "destination": [
                    "obj-42",
                    1
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
              }
            ]
          }
        }
      },
      {
        "box": {
          "id": "obj-44",
          "maxclass": "comment",
          "text": "(transient detection uses spec8 data — bridge.js computes spectral flux)",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            400,
            720,
            380,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-45",
          "maxclass": "comment",
          "text": "TRIGGER MODE",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            40,
            880,
            140,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-46",
          "maxclass": "live.menu",
          "varname": "trigger_mode",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 3,
          "outlettype": [
            "",
            "",
            "float"
          ],
          "patching_rect": [
            40,
            910,
            100,
            15
          ],
          "presentation": 1,
          "presentation_rect": [
            15,
            125,
            110,
            16
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Trigger Mode",
              "parameter_shortname": "Mode",
              "parameter_type": 2,
              "parameter_enum": [
                "TRANSPORT",
                "TRANSIENT",
                "HYBRID"
              ],
              "parameter_initial": [
                2
              ],
              "parameter_initial_enable": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-47",
          "maxclass": "newobj",
          "text": "prepend mode",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            940,
            100,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-48",
          "maxclass": "live.dial",
          "varname": "punch_thresh",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            180,
            910,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            120,
            117,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Punch Thresh",
              "parameter_shortname": "punch_thresh",
              "parameter_type": 0,
              "parameter_mmin": 0.01,
              "parameter_mmax": 1,
              "parameter_initial": [
                0.15
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-49",
          "maxclass": "newobj",
          "text": "prepend punch_thresh",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            180,
            965,
            120,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-50",
          "maxclass": "live.dial",
          "varname": "cut_thresh",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            280,
            910,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            168,
            117,
            44,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Cut Thresh",
              "parameter_shortname": "cut_thresh",
              "parameter_type": 0,
              "parameter_mmin": 0.01,
              "parameter_mmax": 1,
              "parameter_initial": [
                0.45
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-51",
          "maxclass": "newobj",
          "text": "prepend cut_thresh",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            280,
            965,
            120,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-52",
          "maxclass": "comment",
          "text": "MACRO CONTROLS  (Presentation Mode) — MIDI-mappable, saved with the Live Set",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            40,
            320,
            520,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-53",
          "maxclass": "live.dial",
          "varname": "sens",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            40,
            360,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            15,
            15,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Sensitivity",
              "parameter_shortname": "Sensitivity",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 3,
              "parameter_initial": [
                1.3
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-54",
          "maxclass": "newobj",
          "text": "prepend ctrl sens",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            415,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-55",
          "maxclass": "live.dial",
          "varname": "relief",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            135,
            360,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            73,
            15,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Relief",
              "parameter_shortname": "Relief",
              "parameter_type": 0,
              "parameter_mmin": 0.2,
              "parameter_mmax": 2.5,
              "parameter_initial": [
                1.1
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-56",
          "maxclass": "newobj",
          "text": "prepend ctrl relief",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            135,
            415,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-57",
          "maxclass": "live.dial",
          "varname": "agit",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            230,
            360,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            131,
            15,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Agitation",
              "parameter_shortname": "Agitation",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1.5,
              "parameter_initial": [
                0.7
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-58",
          "maxclass": "newobj",
          "text": "prepend ctrl agit",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            230,
            415,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-59",
          "maxclass": "live.dial",
          "varname": "beams",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            325,
            360,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            189,
            15,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Beams",
              "parameter_shortname": "Beams",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1.5,
              "parameter_initial": [
                0.85
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-60",
          "maxclass": "newobj",
          "text": "prepend ctrl beams",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            325,
            415,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-61",
          "maxclass": "live.dial",
          "varname": "punch",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            420,
            360,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            247,
            15,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Punch",
              "parameter_shortname": "Punch",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1.5,
              "parameter_initial": [
                0.85
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-62",
          "maxclass": "newobj",
          "text": "prepend ctrl punch",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            420,
            415,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-63",
          "maxclass": "live.dial",
          "varname": "bloom",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            40,
            455,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            15,
            65,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Bloom",
              "parameter_shortname": "Bloom",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1.5,
              "parameter_initial": [
                0.8
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-64",
          "maxclass": "newobj",
          "text": "prepend ctrl bloom",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            510,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-65",
          "maxclass": "live.dial",
          "varname": "hue",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            135,
            455,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            73,
            65,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Mood (BoC)",
              "parameter_shortname": "Mood (BoC)",
              "parameter_type": 0,
              "parameter_mmin": -20,
              "parameter_mmax": 20,
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-66",
          "maxclass": "newobj",
          "text": "prepend ctrl hue",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            135,
            510,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-67",
          "maxclass": "live.dial",
          "varname": "glitch",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            230,
            455,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            131,
            65,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Glitch",
              "parameter_shortname": "Glitch",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0.12
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-68",
          "maxclass": "newobj",
          "text": "prepend ctrl glitch",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            230,
            510,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-69",
          "maxclass": "live.dial",
          "varname": "gap",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            325,
            455,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            189,
            65,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Tunnel Gap",
              "parameter_shortname": "Tunnel Gap",
              "parameter_type": 0,
              "parameter_mmin": 9,
              "parameter_mmax": 40,
              "parameter_initial": [
                18
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-70",
          "maxclass": "newobj",
          "text": "prepend ctrl gap",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            325,
            510,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-71",
          "maxclass": "live.dial",
          "varname": "scroll",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            420,
            455,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            247,
            65,
            50,
            44
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Scroll Speed",
              "parameter_shortname": "scroll",
              "parameter_type": 0,
              "parameter_mmin": 0.1,
              "parameter_mmax": 1.5,
              "parameter_initial": [
                0.7
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-72",
          "maxclass": "newobj",
          "text": "prepend ctrl scroll",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            420,
            510,
            110,
            22
          ],
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "obj-73",
          "maxclass": "newobj",
          "text": "p visual",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            40,
            590,
            70,
            22
          ],
          "patcher": {
            "fileversion": 1,
            "appversion": {
              "major": 8,
              "minor": 5,
              "revision": 5,
              "architecture": "x64",
              "modernui": 1
            },
            "classnamespace": "box",
            "rect": [
              60,
              80,
              1320,
              840
            ],
            "boxes": [
              {
                "box": {
                  "id": "obj-74",
                  "maxclass": "inlet",
                  "numinlets": 0,
                  "numoutlets": 1,
                  "outlettype": [
                    ""
                  ],
                  "patching_rect": [
                    20,
                    10,
                    30,
                    30
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-75",
                  "maxclass": "newobj",
                  "text": "loadbang",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "outlettype": [
                    "bang"
                  ],
                  "patching_rect": [
                    60,
                    10,
                    70,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-76",
                  "maxclass": "message",
                  "text": "url http://localhost:8765/",
                  "numinlets": 2,
                  "numoutlets": 1,
                  "outlettype": [
                    ""
                  ],
                  "patching_rect": [
                    60,
                    45,
                    200,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-77",
                  "maxclass": "jweb",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    ""
                  ],
                  "patching_rect": [
                    20,
                    90,
                    1280,
                    720
                  ]
                }
              }
            ],
            "lines": [
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
                    "obj-76",
                    0
                  ],
                  "destination": [
                    "obj-77",
                    0
                  ]
                }
              }
            ]
          }
        }
      },
      {
        "box": {
          "id": "obj-78",
          "maxclass": "newobj",
          "text": "pcontrol",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            560,
            70,
            22
          ],
          "outlettype": [
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-79",
          "maxclass": "message",
          "text": "open",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            40,
            530,
            120,
            24
          ],
          "presentation": 1,
          "presentation_rect": [
            222,
            117,
            54,
            24
          ]
        }
      },
      {
        "box": {
          "id": "obj-80",
          "maxclass": "comment",
          "text": "VISUAL WINDOW — click OPEN, drag to HDMI screen",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            170,
            530,
            300,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-81",
          "maxclass": "comment",
          "text": "OPEN -> drag to HDMI",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            170,
            530,
            300,
            20
          ],
          "presentation": 1,
          "presentation_rect": [
            222,
            143,
            130,
            18
          ],
          "textcolor": [
            0.7,
            0.7,
            0.7,
            1
          ]
        }
      }
    ],
    "lines": [
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
            1
          ],
          "destination": [
            "obj-3",
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
            "obj-4",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-2",
            1
          ],
          "destination": [
            "obj-4",
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
            "obj-5",
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
            "obj-6",
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
            "obj-7",
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
            "obj-8",
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
            "obj-10",
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
            "obj-11",
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
            "obj-5",
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
            "obj-5",
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
            "obj-5",
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
            "obj-18",
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
            "obj-5",
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
            "obj-20",
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
            "obj-7",
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
            "obj-26",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            1
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
            "obj-9",
            0
          ],
          "destination": [
            "obj-26",
            1
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            2
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
            "obj-11",
            0
          ],
          "destination": [
            "obj-26",
            2
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            3
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
            0
          ],
          "destination": [
            "obj-26",
            3
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            4
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
            "obj-26",
            4
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            5
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
            "obj-17",
            0
          ],
          "destination": [
            "obj-26",
            5
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            6
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
            "obj-19",
            0
          ],
          "destination": [
            "obj-26",
            6
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-25",
            7
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
            "obj-21",
            0
          ],
          "destination": [
            "obj-26",
            7
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
            "obj-27",
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
            "obj-31",
            3
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
            "obj-31",
            4
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
            "obj-33",
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
            "obj-35",
            0
          ],
          "destination": [
            "obj-37",
            1
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-36",
            2
          ],
          "destination": [
            "obj-37",
            2
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-37",
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
            "obj-38",
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
            "obj-5",
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
            "obj-47",
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
            "obj-48",
            0
          ],
          "destination": [
            "obj-49",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-49",
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
            "obj-50",
            0
          ],
          "destination": [
            "obj-51",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-51",
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
            "obj-53",
            0
          ],
          "destination": [
            "obj-54",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-54",
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
            "obj-55",
            0
          ],
          "destination": [
            "obj-56",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-56",
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
            "obj-57",
            0
          ],
          "destination": [
            "obj-58",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-58",
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
            "obj-59",
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
            "obj-60",
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
            "obj-61",
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
            "obj-62",
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
            "obj-64",
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
            "obj-65",
            0
          ],
          "destination": [
            "obj-66",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-66",
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
            "obj-67",
            0
          ],
          "destination": [
            "obj-68",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-68",
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
            "obj-69",
            0
          ],
          "destination": [
            "obj-70",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "obj-70",
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
            "obj-72",
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
            "obj-79",
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
            "obj-78",
            0
          ],
          "destination": [
            "obj-73",
            0
          ]
        }
      }
    ],
    "dependency_cache": [],
    "autosave": 0
  }
}