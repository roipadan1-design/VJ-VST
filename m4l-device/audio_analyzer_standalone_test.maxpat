{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 0,
			"revision" : 0,
			"architecture" : "x64",
			"modernui" : 1
		},
		"classnamespace" : "box",
		"rect" : [ 60.0, 60.0, 960.0, 780.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"boxes" : [
			{ "box" : { "id" : "obj-1", "maxclass" : "newobj", "text" : "adc~ 1 2", "numinlets" : 0, "numoutlets" : 2, "outlettype" : [ "signal", "signal" ], "patching_rect" : [ 320.0, 20.0, 80.0, 22.0 ] } },
			{ "box" : { "id" : "obj-2", "maxclass" : "newobj", "text" : "+~", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 320.0, 60.0, 50.0, 22.0 ] } },

			{ "box" : { "id" : "obj-3", "maxclass" : "newobj", "text" : "onepole~ 150", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 40.0, 110.0, 100.0, 22.0 ] } },
			{ "box" : { "id" : "obj-4", "maxclass" : "newobj", "text" : "onepole~ 2000", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 220.0, 110.0, 100.0, 22.0 ] } },
			{ "box" : { "id" : "obj-7", "maxclass" : "newobj", "text" : "abs~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 600.0, 110.0, 50.0, 22.0 ] } },

			{ "box" : { "id" : "obj-5", "maxclass" : "newobj", "text" : "-~", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 220.0, 160.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-6", "maxclass" : "newobj", "text" : "-~", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 400.0, 160.0, 50.0, 22.0 ] } },

			{ "box" : { "id" : "obj-8", "maxclass" : "newobj", "text" : "abs~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 40.0, 160.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-9", "maxclass" : "newobj", "text" : "abs~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 220.0, 210.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-10", "maxclass" : "newobj", "text" : "abs~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 400.0, 210.0, 50.0, 22.0 ] } },

			{ "box" : { "id" : "obj-11", "maxclass" : "newobj", "text" : "average~ 200", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 600.0, 160.0, 90.0, 22.0 ] } },
			{ "box" : { "id" : "obj-12", "maxclass" : "newobj", "text" : "average~ 200", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 40.0, 210.0, 90.0, 22.0 ] } },
			{ "box" : { "id" : "obj-13", "maxclass" : "newobj", "text" : "average~ 200", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 220.0, 260.0, 90.0, 22.0 ] } },
			{ "box" : { "id" : "obj-14", "maxclass" : "newobj", "text" : "average~ 200", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "signal" ], "patching_rect" : [ 400.0, 260.0, 90.0, 22.0 ] } },

			{ "box" : { "id" : "obj-15", "maxclass" : "toggle", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "" ], "parameter_enable" : 0, "patching_rect" : [ 780.0, 20.0, 20.0, 20.0 ] } },
			{ "box" : { "id" : "obj-16", "maxclass" : "newobj", "text" : "qmetro 33", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "bang" ], "patching_rect" : [ 780.0, 60.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-17", "maxclass" : "newobj", "text" : "t b b b b", "numinlets" : 1, "numoutlets" : 4, "outlettype" : [ "bang", "bang", "bang", "bang" ], "patching_rect" : [ 780.0, 100.0, 70.0, 22.0 ] } },

			{ "box" : { "id" : "obj-18", "maxclass" : "newobj", "text" : "snapshot~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 600.0, 210.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-19", "maxclass" : "newobj", "text" : "snapshot~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 40.0, 260.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-20", "maxclass" : "newobj", "text" : "snapshot~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 220.0, 310.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-21", "maxclass" : "newobj", "text" : "snapshot~", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 400.0, 310.0, 70.0, 22.0 ] } },

			{ "box" : { "id" : "obj-22", "maxclass" : "newobj", "text" : "* 3.", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 600.0, 260.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-24", "maxclass" : "newobj", "text" : "* 4.", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 40.0, 310.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-26", "maxclass" : "newobj", "text" : "* 4.", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 220.0, 360.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-28", "maxclass" : "newobj", "text" : "* 6.", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 400.0, 360.0, 50.0, 22.0 ] } },

			{ "box" : { "id" : "obj-23", "maxclass" : "newobj", "text" : "clip 0. 1.", "numinlets" : 3, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 600.0, 310.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-25", "maxclass" : "newobj", "text" : "clip 0. 1.", "numinlets" : 3, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 40.0, 360.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-27", "maxclass" : "newobj", "text" : "clip 0. 1.", "numinlets" : 3, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 220.0, 410.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-29", "maxclass" : "newobj", "text" : "clip 0. 1.", "numinlets" : 3, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 400.0, 410.0, 70.0, 22.0 ] } },

			{ "box" : { "id" : "obj-35", "maxclass" : "flonum", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 700.0, 310.0, 60.0, 22.0 ] } },
			{ "box" : { "id" : "obj-36", "maxclass" : "flonum", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 130.0, 360.0, 60.0, 22.0 ] } },
			{ "box" : { "id" : "obj-37", "maxclass" : "flonum", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 320.0, 410.0, 60.0, 22.0 ] } },
			{ "box" : { "id" : "obj-38", "maxclass" : "flonum", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "float" ], "patching_rect" : [ 500.0, 410.0, 60.0, 22.0 ] } },

			{ "box" : { "id" : "obj-30", "maxclass" : "newobj", "text" : "prepend /audio/level", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 600.0, 360.0, 150.0, 22.0 ] } },
			{ "box" : { "id" : "obj-31", "maxclass" : "newobj", "text" : "prepend /audio/bass", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 40.0, 410.0, 150.0, 22.0 ] } },
			{ "box" : { "id" : "obj-32", "maxclass" : "newobj", "text" : "prepend /audio/mid", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 220.0, 460.0, 150.0, 22.0 ] } },
			{ "box" : { "id" : "obj-33", "maxclass" : "newobj", "text" : "prepend /audio/high", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 400.0, 460.0, 150.0, 22.0 ] } },

			{ "box" : { "id" : "obj-42", "maxclass" : "newobj", "text" : "t b l", "numinlets" : 1, "numoutlets" : 2, "outlettype" : [ "bang", "" ], "patching_rect" : [ 600.0, 410.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-43", "maxclass" : "newobj", "text" : "t b l", "numinlets" : 1, "numoutlets" : 2, "outlettype" : [ "bang", "" ], "patching_rect" : [ 40.0, 460.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-44", "maxclass" : "newobj", "text" : "t b l", "numinlets" : 1, "numoutlets" : 2, "outlettype" : [ "bang", "" ], "patching_rect" : [ 220.0, 510.0, 50.0, 22.0 ] } },
			{ "box" : { "id" : "obj-45", "maxclass" : "newobj", "text" : "t b l", "numinlets" : 1, "numoutlets" : 2, "outlettype" : [ "bang", "" ], "patching_rect" : [ 400.0, 510.0, 50.0, 22.0 ] } },

			{ "box" : { "id" : "obj-40", "maxclass" : "newobj", "text" : "OpenSoundControl", "numinlets" : 1, "numoutlets" : 3, "outlettype" : [ "", "", "" ], "patching_rect" : [ 300.0, 580.0, 130.0, 22.0 ] } },
			{ "box" : { "id" : "obj-34", "maxclass" : "newobj", "text" : "udpsend 127.0.0.1 9000", "numinlets" : 1, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 300.0, 630.0, 180.0, 22.0 ] } },

			{ "box" : { "id" : "obj-46", "maxclass" : "newobj", "text" : "loadbang", "numinlets" : 0, "numoutlets" : 1, "outlettype" : [ "bang" ], "patching_rect" : [ 750.0, 360.0, 70.0, 22.0 ] } },
			{ "box" : { "id" : "obj-47", "maxclass" : "message", "text" : "resetallthewaymode 1", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 750.0, 410.0, 150.0, 22.0 ] } },
			{ "box" : { "id" : "obj-48", "maxclass" : "message", "text" : "errorreporting 1", "numinlets" : 2, "numoutlets" : 1, "outlettype" : [ "" ], "patching_rect" : [ 750.0, 460.0, 120.0, 22.0 ] } },

			{ "box" : { "id" : "obj-39", "maxclass" : "comment", "text" : "VJ Audio Analyzer - Phase 0 spike. Click the toggle to start analysis (mic input via adc~). Watch the 4 number boxes react to sound, and check the Max Console for udpsend confirmation. Sends OSC to 127.0.0.1:9000 which the VJ Engine app listens on. To use inside Ableton: swap adc~ for plugin~ and save as an M4L Audio Effect device. Uses the CNMAT OpenSoundControl object (not packOSC, which doesn't exist in current Max) to build outgoing OSC packets.", "numinlets" : 1, "numoutlets" : 0, "patching_rect" : [ 40.0, 690.0, 900.0, 60.0 ] } }
		],
		"lines" : [
			{ "patchline" : { "source" : [ "obj-1", 0 ], "destination" : [ "obj-2", 0 ] } },
			{ "patchline" : { "source" : [ "obj-1", 1 ], "destination" : [ "obj-2", 1 ] } },

			{ "patchline" : { "source" : [ "obj-2", 0 ], "destination" : [ "obj-3", 0 ] } },
			{ "patchline" : { "source" : [ "obj-2", 0 ], "destination" : [ "obj-4", 0 ] } },
			{ "patchline" : { "source" : [ "obj-2", 0 ], "destination" : [ "obj-6", 0 ] } },
			{ "patchline" : { "source" : [ "obj-2", 0 ], "destination" : [ "obj-7", 0 ] } },

			{ "patchline" : { "source" : [ "obj-4", 0 ], "destination" : [ "obj-5", 0 ] } },
			{ "patchline" : { "source" : [ "obj-3", 0 ], "destination" : [ "obj-5", 1 ] } },
			{ "patchline" : { "source" : [ "obj-4", 0 ], "destination" : [ "obj-6", 1 ] } },

			{ "patchline" : { "source" : [ "obj-3", 0 ], "destination" : [ "obj-8", 0 ] } },
			{ "patchline" : { "source" : [ "obj-5", 0 ], "destination" : [ "obj-9", 0 ] } },
			{ "patchline" : { "source" : [ "obj-6", 0 ], "destination" : [ "obj-10", 0 ] } },

			{ "patchline" : { "source" : [ "obj-7", 0 ], "destination" : [ "obj-11", 0 ] } },
			{ "patchline" : { "source" : [ "obj-8", 0 ], "destination" : [ "obj-12", 0 ] } },
			{ "patchline" : { "source" : [ "obj-9", 0 ], "destination" : [ "obj-13", 0 ] } },
			{ "patchline" : { "source" : [ "obj-10", 0 ], "destination" : [ "obj-14", 0 ] } },

			{ "patchline" : { "source" : [ "obj-15", 0 ], "destination" : [ "obj-16", 0 ] } },
			{ "patchline" : { "source" : [ "obj-16", 0 ], "destination" : [ "obj-17", 0 ] } },

			{ "patchline" : { "source" : [ "obj-11", 0 ], "destination" : [ "obj-18", 0 ] } },
			{ "patchline" : { "source" : [ "obj-17", 0 ], "destination" : [ "obj-18", 0 ] } },
			{ "patchline" : { "source" : [ "obj-12", 0 ], "destination" : [ "obj-19", 0 ] } },
			{ "patchline" : { "source" : [ "obj-17", 1 ], "destination" : [ "obj-19", 0 ] } },
			{ "patchline" : { "source" : [ "obj-13", 0 ], "destination" : [ "obj-20", 0 ] } },
			{ "patchline" : { "source" : [ "obj-17", 2 ], "destination" : [ "obj-20", 0 ] } },
			{ "patchline" : { "source" : [ "obj-14", 0 ], "destination" : [ "obj-21", 0 ] } },
			{ "patchline" : { "source" : [ "obj-17", 3 ], "destination" : [ "obj-21", 0 ] } },

			{ "patchline" : { "source" : [ "obj-18", 0 ], "destination" : [ "obj-22", 0 ] } },
			{ "patchline" : { "source" : [ "obj-22", 0 ], "destination" : [ "obj-23", 0 ] } },
			{ "patchline" : { "source" : [ "obj-19", 0 ], "destination" : [ "obj-24", 0 ] } },
			{ "patchline" : { "source" : [ "obj-24", 0 ], "destination" : [ "obj-25", 0 ] } },
			{ "patchline" : { "source" : [ "obj-20", 0 ], "destination" : [ "obj-26", 0 ] } },
			{ "patchline" : { "source" : [ "obj-26", 0 ], "destination" : [ "obj-27", 0 ] } },
			{ "patchline" : { "source" : [ "obj-21", 0 ], "destination" : [ "obj-28", 0 ] } },
			{ "patchline" : { "source" : [ "obj-28", 0 ], "destination" : [ "obj-29", 0 ] } },

			{ "patchline" : { "source" : [ "obj-23", 0 ], "destination" : [ "obj-30", 0 ] } },
			{ "patchline" : { "source" : [ "obj-25", 0 ], "destination" : [ "obj-31", 0 ] } },
			{ "patchline" : { "source" : [ "obj-27", 0 ], "destination" : [ "obj-32", 0 ] } },
			{ "patchline" : { "source" : [ "obj-29", 0 ], "destination" : [ "obj-33", 0 ] } },

			{ "patchline" : { "source" : [ "obj-23", 0 ], "destination" : [ "obj-35", 0 ] } },
			{ "patchline" : { "source" : [ "obj-25", 0 ], "destination" : [ "obj-36", 0 ] } },
			{ "patchline" : { "source" : [ "obj-27", 0 ], "destination" : [ "obj-37", 0 ] } },
			{ "patchline" : { "source" : [ "obj-29", 0 ], "destination" : [ "obj-38", 0 ] } },

			{ "patchline" : { "source" : [ "obj-30", 0 ], "destination" : [ "obj-42", 0 ] } },
			{ "patchline" : { "source" : [ "obj-31", 0 ], "destination" : [ "obj-43", 0 ] } },
			{ "patchline" : { "source" : [ "obj-32", 0 ], "destination" : [ "obj-44", 0 ] } },
			{ "patchline" : { "source" : [ "obj-33", 0 ], "destination" : [ "obj-45", 0 ] } },

			{ "patchline" : { "source" : [ "obj-42", 0 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-42", 1 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-43", 0 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-43", 1 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-44", 0 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-44", 1 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-45", 0 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-45", 1 ], "destination" : [ "obj-40", 0 ] } },

			{ "patchline" : { "source" : [ "obj-40", 0 ], "destination" : [ "obj-34", 0 ] } },

			{ "patchline" : { "source" : [ "obj-46", 0 ], "destination" : [ "obj-47", 0 ] } },
			{ "patchline" : { "source" : [ "obj-46", 0 ], "destination" : [ "obj-48", 0 ] } },
			{ "patchline" : { "source" : [ "obj-47", 0 ], "destination" : [ "obj-40", 0 ] } },
			{ "patchline" : { "source" : [ "obj-48", 0 ], "destination" : [ "obj-40", 0 ] } }
		]
	}
}
