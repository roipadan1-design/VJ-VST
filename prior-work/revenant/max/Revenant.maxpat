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
      1120,
      720
    ],
    "openrect": [
      0,
      0,
      0,
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
    "title": "REVENANT",
    "boxes": [
      {
        "box": {
          "id": "obj-1",
          "maxclass": "comment",
          "text": "REVENANT — audio analysis -> Node -> WebSocket -> visual (auto-opens). Macro knobs are MIDI-mappable.",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            30,
            8,
            640,
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
            560,
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
          "text": "metro 10",
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
            300,
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
          "text": "bridge.js: serves ../dist on :8765, broadcasts analysis + macro ctrl over WebSocket",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            700,
            500,
            420,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-30",
          "maxclass": "comment",
          "text": "MACRO CONTROLS (Presentation) — right-click a knob → MIDI-map. Saved with the Live Set.",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            40,
            300,
            560,
            20
          ]
        }
      },
      {
        "box": {
          "id": "obj-31",
          "maxclass": "live.dial",
          "varname": "react",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            40,
            340,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            20,
            20,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Reactivity",
              "parameter_shortname": "Reactivity",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 2,
              "parameter_initial": [
                1
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-32",
          "maxclass": "newobj",
          "text": "prepend ctrl react",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            395,
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
          "id": "obj-33",
          "maxclass": "live.dial",
          "varname": "disint",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            135,
            340,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            100,
            20,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Disintegrate",
              "parameter_shortname": "disint",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 2,
              "parameter_initial": [
                0.9
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-34",
          "maxclass": "newobj",
          "text": "prepend ctrl disint",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            135,
            395,
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
          "id": "obj-35",
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
            340,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            180,
            20,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Glitch",
              "parameter_shortname": "Glitch",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 2,
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
          "id": "obj-36",
          "maxclass": "newobj",
          "text": "prepend ctrl glitch",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            230,
            395,
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
          "id": "obj-37",
          "maxclass": "live.dial",
          "varname": "blue",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            325,
            340,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            260,
            20,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Blue Flash",
              "parameter_shortname": "Blue Flash",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 2,
              "parameter_initial": [
                0.2
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-38",
          "maxclass": "newobj",
          "text": "prepend ctrl blue",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            325,
            395,
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
          "id": "obj-39",
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
            435,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            20,
            84,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Bloom",
              "parameter_shortname": "Bloom",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 3,
              "parameter_initial": [
                0.95
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-40",
          "maxclass": "newobj",
          "text": "prepend ctrl bloom",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            490,
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
          "id": "obj-41",
          "maxclass": "live.dial",
          "varname": "rotate",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            135,
            435,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            100,
            84,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Rotation",
              "parameter_shortname": "Rotation",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1.2,
              "parameter_initial": [
                0.22
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-42",
          "maxclass": "newobj",
          "text": "prepend ctrl rotate",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            135,
            490,
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
          "id": "obj-43",
          "maxclass": "live.dial",
          "varname": "grain",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            230,
            435,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            180,
            84,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Grain",
              "parameter_shortname": "Grain",
              "parameter_type": 0,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                1
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-44",
          "maxclass": "newobj",
          "text": "prepend ctrl grain",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            230,
            490,
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
          "id": "obj-45",
          "maxclass": "live.dial",
          "varname": "transient",
          "parameter_enable": 1,
          "numinlets": 1,
          "numoutlets": 2,
          "outlettype": [
            "",
            "float"
          ],
          "patching_rect": [
            325,
            435,
            60,
            48
          ],
          "presentation": 1,
          "presentation_rect": [
            260,
            84,
            56,
            48
          ],
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Transient",
              "parameter_shortname": "Transient",
              "parameter_type": 0,
              "parameter_mmin": 0.2,
              "parameter_mmax": 3,
              "parameter_initial": [
                1
              ],
              "parameter_initial_enable": 1,
              "parameter_unitstyle": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "obj-46",
          "maxclass": "newobj",
          "text": "prepend ctrl transient",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            325,
            490,
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
          "id": "obj-47",
          "maxclass": "newobj",
          "text": "p visual",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            40,
            470,
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
              0,
              0,
              1600,
              900
            ],
            "openinpresentation": 1,
            "boxes": [
              {
                "box": {
                  "id": "obj-48",
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
                  "id": "obj-49",
                  "maxclass": "newobj",
                  "text": "loadbang",
                  "numinlets": 1,
                  "numoutlets": 1,
                  "outlettype": [
                    "bang"
                  ],
                  "patching_rect": [
                    70,
                    10,
                    70,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-50",
                  "maxclass": "newobj",
                  "text": "del 1500",
                  "numinlets": 2,
                  "numoutlets": 1,
                  "outlettype": [
                    "bang"
                  ],
                  "patching_rect": [
                    70,
                    45,
                    70,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-51",
                  "maxclass": "newobj",
                  "text": "del 3500",
                  "numinlets": 2,
                  "numoutlets": 1,
                  "outlettype": [
                    "bang"
                  ],
                  "patching_rect": [
                    150,
                    45,
                    70,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-52",
                  "maxclass": "message",
                  "text": "url http://localhost:8765/",
                  "numinlets": 2,
                  "numoutlets": 1,
                  "outlettype": [
                    ""
                  ],
                  "patching_rect": [
                    20,
                    80,
                    220,
                    22
                  ]
                }
              },
              {
                "box": {
                  "id": "obj-53",
                  "maxclass": "jweb",
                  "varname": "revweb",
                  "numinlets": 1,
                  "numoutlets": 2,
                  "outlettype": [
                    "",
                    ""
                  ],
                  "patching_rect": [
                    20,
                    120,
                    1280,
                    720
                  ],
                  "presentation": 1,
                  "presentation_rect": [
                    0,
                    0,
                    1600,
                    900
                  ]
                }
              }
            ],
            "lines": [
              {
                "patchline": {
                  "source": [
                    "obj-48",
                    0
                  ],
                  "destination": [
                    "obj-52",
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
                    "obj-52",
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
                    "obj-50",
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
                    "obj-52",
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
                    "obj-52",
                    0
                  ]
                }
              },
              {
                "patchline": {
                  "source": [
                    "obj-52",
                    0
                  ],
                  "destination": [
                    "obj-53",
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
          "id": "obj-54",
          "maxclass": "newobj",
          "text": "pcontrol",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            40,
            440,
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
          "id": "obj-55",
          "maxclass": "message",
          "text": "open",
          "numinlets": 2,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            40,
            410,
            60,
            22
          ],
          "presentation": 1,
          "presentation_rect": [
            20,
            150,
            130,
            26
          ]
        }
      },
      {
        "box": {
          "id": "obj-56",
          "maxclass": "newobj",
          "text": "loadbang",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            180,
            410,
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
          "id": "obj-57",
          "maxclass": "newobj",
          "text": "deferlow",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            180,
            438,
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
          "id": "obj-58",
          "maxclass": "newobj",
          "text": "delay 2500",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            180,
            466,
            80,
            22
          ],
          "outlettype": [
            "bang"
          ]
        }
      },
      {
        "box": {
          "id": "obj-59",
          "maxclass": "comment",
          "text": "VISUAL auto-opens ~2.5s after load. Drag the window to your HDMI screen, press F for fullscreen.",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            130,
            410,
            360,
            20
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
            "obj-28",
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
            "obj-28",
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
            "obj-39",
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
            "obj-40",
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
            "obj-42",
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
            "obj-28",
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
            "obj-54",
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
            "obj-55",
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
            "obj-56",
            0
          ],
          "destination": [
            "obj-57",
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
            "obj-54",
            0
          ]
        }
      }
    ],
    "dependency_cache": [],
    "autosave": 0
  }
}