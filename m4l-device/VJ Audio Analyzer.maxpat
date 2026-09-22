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
		"devicewidth": 0.0,
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
						42.0,
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
						40.0,
						50.0,
						20.0
					]
				}
			},
			{
				"box": {
					"id": "obj-90",
					"maxclass": "comment",
					"text": "VJ AUDIO ANALYZER",
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
						4.0,
						260.0,
						18.0
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
						24.0,
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
						40.0,
						18.0,
						110.0
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
						24.0,
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
						40.0,
						18.0,
						110.0
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
						24.0,
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
						40.0,
						18.0,
						110.0
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
						24.0,
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
						40.0,
						18.0,
						110.0
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
						24.0,
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
						24.0,
						50.0,
						14.0
					]
				}
			},
			{
				"box": {
					"id": "obj-39",
					"maxclass": "comment",
					"text": "VJ Audio Analyzer - M4L Audio Effect. Analyzes level/bass/mid/high + Live's beat phase, sends OSC to 127.0.0.1:9000 for the VJ Engine render process. Audio passes through unaffected (plugin~ -> plugout~). Toggle freezes/resumes analysis; auto-starts on load. Preset Select/Next/Prev drive the VJ Engine's active preset over the same OSC connection. Front end is in Presentation mode (right-click the device title, or the view icon, to switch between Presentation and the raw patch shown here).",
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
					"presentation": 1,
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
						80.0,
						175.0,
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
						80.0,
						159.0,
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
						110.0,
						175.0,
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
						110.0,
						159.0,
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
						159.0,
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