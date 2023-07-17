package main

import (
	"testing"
	"reflect"
	//"math"
)

type testUMData struct{
	input terminal
	onstate int
	power float64
	vdc float64
}

type testGLLData struct{
	input terminal
	gridforming bool
	result terminal
	output terminal
	voltageExternal terminal
}

type testDVData struct{
	input terminal
	gridforming bool
	onstate bool
	output terminal
	Vdc1cmd float64
	vdc1internal float64
}

type testCSData struct{
	input terminal
	gridforming bool
	pcmd float64
	onstate bool
	output terminal
}

type testDLData struct{
	input terminal
	Pdc1 float64
	output terminal
}

type testUSData struct{
	input terminal
	plim float64
	pcmd float64
	idleloss float64
	pesr float64
	DVdc2 droop
	output terminal
}

func TestDCDCOnOff(t *testing.T) {
	d := dcdc{
		ID: "dcdc",
	}
	input := terminal{
		v: 480,
	}

	d.Init()

	d.Oncmd = true

	d.UpdateMode(input)
	ess1.DistributeVoltage(input)
	ess1.UpdateState(input, 1)

	if d.Vdc2 != input.vdc {
		t.Errorf("dcdc voltage incorrect, got %f but expected %f", d.Vdc1, input.vdc)
	}
}

func TestUpdateMode(t *testing.T) {
	d := dcdc{
		ID: "dcdc",
	}
	d.Init()

	tests := []testUMData{
		testUMData{
			input : terminal{
			},
			onstate: 1,
			power: 0.0,
			vdc: 0.0,
		},
		testUMData{
			input: terminal{
				p: 300.2, vdc:1023,
			},
			onstate: 1,
			power: 300.2,
			vdc: 1023,
		},
		testUMData{
			input: terminal{
				p: -300.2, vdc:0.2222,
			},
			onstate: 1,
			power: -300.2,
			vdc: 0.2222,
		},
		testUMData{
			input: terminal{
				p: 300.2, vdc:1023,
			},
			onstate: 0,
			power: 0,
			vdc: 0,
		},
	}
	for _,test:=range tests{
		d.On = (test.onstate == 1)
		d.UpdateMode(test.input)
		if(d.Pdc2 != test.power || d.Vdc2 != test.vdc){
			t.Errorf("FAILED. Expected power: %f, Expected voltage: %f. Actual power: %f, voltage: %f\n",test.power,test.vdc,d.Pdc2,d.Vdc2)
		}
	}
}

func TestGetLoadLines(t * testing.T){
	d := dcdc{
		ID: "dcdc",
	}
	d.Init()

	DVdc1 := droop{
		Percent: 0.9,
		YNom: 3000,
		XNom: 1326,
	}
	DVdc2 := droop{
		Percent: 0.9,
		YNom: 2000,
		XNom: 480,
	}
	dcterminal := terminal{
		p: 1000,
		dVdc: DVdc1,
	}
	d.On = true

	tests := []testGLLData{
		testGLLData{
			input: terminal{},
			gridforming: false,
			output: terminal{},
		},
		testGLLData{
			input: terminal{p: 1000, vdc: 480,
				dVdc: droop{
					slope: -6000, offset: 3333,
				},
			},
			gridforming: true,
			output: terminal{
				dVdc: DVdc2,
			},
		},
	}

	d.DVdc2 = DVdc2
	d.DVdc1 = DVdc1
	for _,test := range tests{
		d.GridForming = test.gridforming
		test.result=d.GetLoadLines(test.input, 0.1)
		test.voltageExternal= combineTerminals(dcterminal, test.input)
		if!(reflect.DeepEqual(test.result, test.output)){
			t.Errorf("Incorrect output. Expected %v Actual %v\n", test.output, test.result)
		}
		// if!(reflect.DeepEqual(d.DvoltageExternal, test.voltageExternal)){
		// 	t.Errorf("Incorrect external voltage. Expected %v Actual %v\n",test.voltageExternal, d.DvoltageExternal)
		// }
	}
}

func TestDistributeVoltage(t * testing.T) {
	d := dcdc{
		ID: "dcdc",
	}
	d.Init()

	DVdc2 := droop{
		Percent: 0.9,
		YNom: 2000,
		XNom: 480,
	}
	DVoltageExternal := droop{
		slope: -1000,
		offset: 3333,
	}
	dummy := terminal{
		p: 100.0,
		vdc: 1000.0,
		dVdc: droop{
			slope: -123.4,
			offset: 5678,
		},
	}
	tests:=[]testDVData{
		testDVData{
			input: terminal{},
			gridforming: true,
			onstate: true,
			Vdc1cmd: 100.0,
			vdc1internal: 300.0,
			output: terminal{
				vdc: 0,
				dVdc: droop{},
			},
		},
		testDVData{
			input: terminal{
				vdc: 100.0,
			},
			gridforming: true,
			onstate: true,
			Vdc1cmd: 100.0,
			vdc1internal: 300.0,
			output: terminal{
				vdc: 100.0,
				dVdc: DVoltageExternal,
			},
		},
		testDVData{
			input: terminal{
				vdc: 100.0,
			},
			gridforming: false,
			onstate: true,
			Vdc1cmd: 100.0,
			vdc1internal: 300.0,
			output: terminal{
				vdc: 300.0,
				dVdc: DVoltageExternal,
			},
		},
		testDVData{
			input: terminal{
				vdc: 100.0,
			},
			gridforming: false,
			onstate: false,
			Vdc1cmd: 100.0,
			vdc1internal: 300.0,
			output: terminal{
				vdc: 0,
				dVdc: droop{},
			},
		},
		testDVData{
			input: dummy,
			gridforming: true,
			onstate: true,
			Vdc1cmd: 100.0,
			vdc1internal: 300.0,
			output: terminal{
				vdc: 100.0,
				dVdc: DVoltageExternal,
			},
		},
		testDVData{
			input: dummy,
			gridforming: false,
			onstate: true,
			Vdc1cmd: 100.0,
			vdc1internal: 300.0,
			output: terminal{
				vdc: 300.0,
				dVdc: DVoltageExternal,
			},
		},
	}

	d.DVdc2 = DVdc2
	d.DvoltageExternal = DVoltageExternal
	for i,test := range tests{
		d.On = test.onstate
		d.GridForming = test.gridforming
		d.Vdc1 = test.vdc1internal
		d.VDC1cmd = test.Vdc1cmd
		result := d.DistributeVoltage(test.input)
		if !(reflect.DeepEqual(result, test.output)){
			t.Errorf("[Case %d] Unexpected output. Expected %v Actual %v\n", i, test.output, result)
		}
	}
}

func TestCalculateState(t *testing.T){
	d := dcdc{
		ID: "dcdc",
	}
	d.Plim = 3000.0
	d.Idleloss = 5.5
	d.Rte = 98.0
	d.Init()
	dummy := terminal{
		p: 100.0,
		vdc: 1000.0,
		dVdc: droop{
			slope: -123.4,
			offset: 5678,
		},
	}
	tests:= []testCSData{
		testCSData{
			input: terminal{},
			gridforming: false,
			onstate: true,
			pcmd: 300.0,
			output: terminal{},
		},
		testCSData{
			input: dummy,
			gridforming: true,
			onstate: true,
			pcmd: -300.0,
			output: terminal{},
		},
		testCSData{
			input: dummy,
			gridforming: false,
			onstate: false,
			pcmd: 300.0,
			output: terminal{},
		},
		testCSData{
			input: dummy,
			gridforming: false,
			onstate: true,
			pcmd: -300.0,
			output: terminal{},
		},
	}
	d.Noise2 = 0.0
	for i,test := range tests{
		if !test.gridforming && test.onstate{
			test.output.p = test.pcmd /*+ d.Idleloss + d.pesr*math.Pow(test.pcmd, 2)*/
		}
		d.On = test.onstate
		d.GridForming = test.gridforming
		d.Pcmd = test.pcmd
		result := d.CalculateState(test.input, 0.01)
		if !(reflect.DeepEqual(result, test.output)){
			t.Errorf("[Case %d]Unexpected output. Expected %v\n Actual %v",i, test.output, result)
		}
	}
}

func TestDistributeLoad(t *testing.T){
	d := dcdc{
		ID: "dcdc",
	}
	d.Init()

	voltageExternal := droop{
		slope: -500, offset: 3333,
	}

	dummy := terminal{
		p: 100.0,
		vdc: 1000.0,
		dVdc: droop{
			slope: -123.4,
			offset: 5678,
		},
	}

	tests:= []testDLData{
		testDLData{
			input: terminal{},
			Pdc1: 300.,
			output: terminal{
				vdc: getX(0, voltageExternal.slope, voltageExternal.offset),
				p: 300.0,
			},
		},
		testDLData{
			input: dummy,
			Pdc1: 400.0,
			output: terminal{
				vdc: getX(100, voltageExternal.slope, voltageExternal.offset),
				p: 400.0,
			},
		},
	}
	for i,test := range tests{
		d.Pdc1 = test.Pdc1
		d.DvoltageExternal = voltageExternal
		result := d.DistributeLoad(test.input)
		if !(reflect.DeepEqual(result, test.output)){
			t.Errorf("[Case %d] Expected %v actual %v\n",i, test.output,result)
		}
	}
}

func TestUpdateState(t *testing.T){
	d := dcdc{
		ID: "dcdc",
	}
	d.Init()
	//cases
	//zero input
	//nominal input
	//pcmd > plim
	//input p < pdel
	nominaldroop := droop{
		XNom: 1300, YNom: 1000, Percent: 0.9,
	}
	nominaldroop.slope, nominaldroop.offset = getSlope(nominaldroop.Percent, nominaldroop.YNom, nominaldroop.XNom)

	tests:= []testUSData{
		testUSData{
			input: terminal{p: 0,},
			plim: 1000.0,
			pcmd: 500.0,
			idleloss: 5.5,
			pesr: 0.000005,
			DVdc2: nominaldroop,
			output: terminal{
				p: 0.0, vdc: 1300, dVdc: nominaldroop,
			},
		},
		testUSData{
			input: terminal{p:3000.0},
			plim: 1000.0,
			pcmd: 500.0,
			idleloss: 5.5,
			pesr: 0.000005,
			DVdc2: nominaldroop,
			output: terminal{
				p: 500.0, vdc: 1300, dVdc: nominaldroop,
			},
		},
		testUSData{
			input: terminal{p:300.0},
			plim: 1000.0,
			pcmd: 1500.0,
			idleloss: 5.5,
			pesr: 0.000005,
			DVdc2: nominaldroop,
			output: terminal{
				p: 1000.0, vdc: 1300, dVdc: nominaldroop,
			},
		},
		testUSData{
			input: terminal{p:300.0},
			plim: 1000.0,
			pcmd: 500.0,
			idleloss: 5.5,
			pesr: 0.000005,
			DVdc2: nominaldroop,
			output: terminal{
				p: 500.0, vdc: 1300, dVdc: nominaldroop,
			},
		},
	}
	d.On = true
	for i,test := range tests{
		d.Plim = test.plim
		d.Pcmd = test.pcmd
		d.Idleloss = test.idleloss
		d.pesr = test.pesr
		d.DVdc2 = test.DVdc2
		result := d.UpdateState(test.input, 0.01)
		if !(reflect.DeepEqual(result, test.output)){
			t.Errorf("[Case %d] Expected %v Actual %v\n", i, test.output, result)
		}
	}
}