package main

import (
	"testing"
)

var ess1, ess2 ess
var gen1 gen
var pv1, pv2 pv
var feedSite, feedEss, feedPv feed
var txEss, txPv xfmr
var loadMv, loadLv load
var grid1 grid

var assetList = []asset{&grid1, &feedSite, &feedEss, &feedPv,
	&ess1, &ess2, &pv1, &pv2, &txEss, &txPv, &loadMv, &loadLv,
}

func resetTree() {
	ess1 = ess{
		ID:    "ess_1",
		Cap:   1000,
		Soc:   50,
		Phigh: 500,
		Plow:  500,
		Oncmd: true,
	}
	ess2 = ess{
		ID:    "ess_2",
		Cap:   1000,
		Soc:   50,
		Phigh: 500,
		Plow:  500,
		Oncmd: true,
	}
	gen1 = gen{
		ID:    "gen_1",
		Pramp: 100,
		Qramp: 100,
		Oncmd: true,
	}
	pv1 = pv{
		ID:    "pv_1",
		Pramp: 100,
		Qramp: 100,
		Phigh: 125,
		Pcmd:  100,
		Oncmd: true,
	}
	pv2 = pv{
		ID:    "pv_2",
		Pramp: 100,
		Qramp: 100,
		Phigh: 125,
		Pcmd:  100,
		Oncmd: true,
	}
	feedSite = feed{
		ID:       "feed_site",
		Pmax:     2000,
		Qmax:     2000,
		Smax:     2000,
		Closecmd: true,
	}
	feedEss = feed{
		ID:       "feed_ess",
		Pmax:     2000,
		Qmax:     2000,
		Smax:     2000,
		Closecmd: true,
	}
	feedPv = feed{
		ID:       "feed_pv",
		Pmax:     2000,
		Qmax:     2000,
		Smax:     2000,
		Closecmd: true,
	}
	txEss = xfmr{
		ID:  "tx_ess",
		Vn:  12470,
		Sn:  1250,
		N:   26,
		Zpu: 1,
		XoR: 10,
		Eff: 99.999,
	}
	txPv = xfmr{
		ID:  "tx_pv",
		Vn:  12470,
		Sn:  1250,
		N:   32,
		Zpu: 1,
		XoR: 10,
		Eff: 99.999,
	}
	loadMv = load{
		ID:    "load_mv",
		Pramp: 100,
		Qramp: 100,
		Pcmd:  -500,
		Qcmd:  -100,
		Oncmd: true,
	}
	loadLv = load{
		ID:    "load_lv",
		Pramp: 100,
		Qramp: 100,
		Pcmd:  -200,
		Qcmd:  -50,
		Oncmd: true,
	}
	grid1 = grid{
		ID: "grid",
		V:  12470,
		F:  60,
	}
	// for _, a := range assetList {
	// 	a.Init()
	// } // This probably needs to be done but currently breaks tests
	// It probably adds some of the undesireable losses that make comparisons hard
}

func TestGridFollowing(t *testing.T) {
	t.Log("TWINs Test Begins")
	var rootNode = treeNode{
		asset: &grid1,
		children: []treeNode{
			treeNode{
				asset: &feedSite,
				children: []treeNode{
					treeNode{
						asset: &feedEss,
						children: []treeNode{
							treeNode{
								asset: &txEss,
								children: []treeNode{
									treeNode{
										asset:    &ess1,
										children: []treeNode{},
									},
									treeNode{
										asset:    &ess2,
										children: []treeNode{},
									},
								},
							},
						},
					},
					treeNode{
						asset:    &loadMv,
						children: []treeNode{},
					},
					treeNode{
						asset: &feedPv,
						children: []treeNode{
							treeNode{
								asset: &txPv,
								children: []treeNode{
									treeNode{
										asset:    &pv1,
										children: []treeNode{},
									},
									treeNode{
										asset:    &pv2,
										children: []treeNode{},
									},
									treeNode{
										asset:    &loadLv,
										children: []treeNode{},
									},
								},
							},
						},
					},
				},
			},
		},
	}

	resetTree()
	grid1.Init()
	// Test loads only
	discoverTree(rootNode, rootNode.asset.Term(), 600)
	calculateTree(rootNode, rootNode.asset.Term(), 600)
	output := updateTree(rootNode, rootNode.asset.Term(), 600)
	if !(output.p == -700 && output.q == -150) {
		t.Log("Loads only")
		t.Logf("Got %.2fkW, %.2fkVAR, expected -700kW, -150kVAR", output.p, output.q)
		t.Log(gen1.V)
		t.Log(feedPv.P, feedPv.V1, feedPv.V2)
		t.Log(txPv.P, txPv.V1, txPv.V2)
		t.Log(loadLv.P, loadLv.V)
		t.Fail()
	}

	// Turn on devices and command them
	ess1.Oncmd, ess2.Oncmd, pv1.Oncmd, pv2.Oncmd = true, true, true, true
	ess1.Pcmd, ess2.Pcmd, pv1.Plim, pv2.Plim = 350, 350, 50, 50
	ess1.Qcmd, ess2.Qcmd = 100, 100
	discoverTree(rootNode, rootNode.asset.Term(), 600)
	calculateTree(rootNode, rootNode.asset.Term(), 600)
	output = updateTree(rootNode, rootNode.asset.Term(), 600)
	if !(output.p == 100 && output.q == 50) {
		t.Log("ESS and PV")
		t.Logf("Got %.2fkW, %.2fkVAR, expected 100kW, 50kVAR", output.p, output.q)
		t.Fail()
	}

	// Open site feeder, make sure devices turn off
	feedSite.Opencmd = true
	discoverTree(rootNode, rootNode.asset.Term(), 600)
	calculateTree(rootNode, rootNode.asset.Term(), 600)
	output = updateTree(rootNode, rootNode.asset.Term(), 600)
	if !(output.p == 0 && output.q == 0) {
		t.Log("Site feed open")
		t.Logf("Got %.2fkW, %.2fkVAR, expected 0kW, 0kVAR", output.p, output.q)
		t.Fail()
	}
	if !(feedSite.Closed == false && ess1.On == false && ess2.On == false && pv1.On == false && pv2.On == false &&
		loadMv.On == false && loadLv.On == false && feedEss.Closed == true && feedPv.Closed == true) {
		t.Log("Site feed open")
		t.Log("Something failed to open or turn off:", feedSite.Closed, ess1.On, ess2.On, pv1.On, pv2.On,
			loadMv.On, loadLv.On)
		t.Log("Or something failed to stay closed", feedEss.Closed, feedPv.Closed)
		t.Log(loadMv.V, feedSite.V2, feedSite.V1)
		t.Fail()
	}

	// Make sure reclosure and turning back on works
	feedSite.Closecmd = true
	ess1.Oncmd, ess2.Oncmd, pv1.Oncmd, pv2.Oncmd, loadMv.Oncmd, loadLv.Oncmd = true, true, true, true, true, true
	discoverTree(rootNode, rootNode.asset.Term(), 600)
	calculateTree(rootNode, rootNode.asset.Term(), 600)
	output = updateTree(rootNode, rootNode.asset.Term(), 600)
	if !(output.p == 100 && output.q == 50) {
		t.Log("Site feed reclosed")
		t.Logf("Got %.2fkW, %.2fkVAR, expected 100kW, 50kVAR", output.p, output.q)
		t.Fail()
	}

	// Test that a parameter can be changed and the devices can be re-initialized
	t.Logf("Configured transformer efficiency is %f\n", txEss.Eff)
	t.Logf("Configured transformer no load power is %f\n", txEss.pnoload)
	txEss.Eff = 99.5
	txPv.Eff = 99.5
	txEss.Mag = 1
	t.Logf("Updated transformer efficiency is %f\n", txEss.Eff)
	t.Logf("Transformer no load power remains %f\n", txEss.pnoload)
	initTree(rootNode)
	t.Logf("Re-initialized transformer efficiency is %f\n", txEss.Eff)
	t.Logf("Re-initialized transformer no load power is %f\n", txEss.pnoload)
}

func TestGridForming(t *testing.T) {
	testRoot := treeNode{
		asset: &grid1,
		children: []treeNode{
			treeNode{
				asset: &txEss,
				children: []treeNode{
					treeNode{
						asset: &feedSite,
						children: []treeNode{
							treeNode{asset: &ess1},
							treeNode{asset: &gen1},
							treeNode{asset: &loadLv},
						},
					},
				},
			},
		},
	}

	// Test load only supplied by ESS on 480V bus
	// Generator is off
	// Grid is disconnected
	resetTree()
	grid1.Init()
	ess1.GridFormingCmd = true
	ess1.Dactive = droop{YNom: 1000, Percent: 0.05, XNom: 60}
	ess1.Fcmd = 60
	ess1.Dreactive = droop{YNom: 100, Percent: 0.10, XNom: 480}
	ess1.Vcmd = 480
	ess1.Init()
	gen1.GridFormingCmd = false
	gen1.Dactive = droop{YNom: 1000, Percent: 0.05, XNom: 60}
	gen1.Fcmd = 60
	gen1.Dreactive = droop{YNom: 100, Percent: 0.10, XNom: 480}
	gen1.Vcmd = 480
	gen1.Init()

	// Test load only supplied by ESS on 480V bus
	// Grid is disconnected
	feedSite.Closecmd = false
	// First go is to get the output set
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	// Second iteration is to let droop take effect
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	if !(ess1.P == 200 && grid1.P == 0 && loadLv.P == -200) {
		t.Log("Test 1: Grid forming - one ess, load only")
		t.Logf("Got ESS output %.1fkW, grid output %.1fkW, load output %.1fkW", ess1.P, grid1.P, loadLv.P)
		t.Log("Expected 200kW, 0kW, -200kW")
		t.Fail()
	}
	if !(floatEq(loadLv.V, 456, 0.1) && floatEq(loadLv.F, 59.4, 0.01)) {
		t.Log("Test 1: Grid forming - one ess, load only")
		t.Logf("Got load voltage %.1fV, load frequency %.3f", loadLv.V, loadLv.F)
		t.Log("Expected 456V, 59.4Hz")
		t.Fail()
	}
	// Test load only supplied by both ESS and Generator on 480V bus
	// Equal droop settings
	// Grid is disconnected
	gen1.GridFormingCmd = true
	// First go is to get the output set
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	// Second iteration is to let droop take effect
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	if !(ess1.P == 100 && gen1.P == 100 && grid1.P == 0 && loadLv.P == -200) {
		t.Log("Test 2: Grid forming - one ess, one gen, load only")
		t.Logf("Got ESS output %.1fkW, grid output %.1fkW, load output %.1fkW", ess1.P, grid1.P, loadLv.P)
		t.Log("Expected 100kW, 100kW, 0kW, -200kW")
		t.Fail()
	}
	if !(floatEq(loadLv.V, 468, 0.1) && floatEq(loadLv.F, 59.7, 0.01)) {
		t.Log("Test 2: Grid forming - one ess, one gen, load only")
		t.Logf("Got load voltage %.1fV, load frequency %.3f", loadLv.V, loadLv.F)
		t.Log("Expected 468V, 59.7Hz")
		t.Fail()
	}

	// Grid is connected to supply the test load
	// ESS and Generator should output very little
	feedSite.Closecmd = true
	// First go is to get the output set
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	// Second iteration is to let droop take effect
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	if !(floatEq(ess1.P, 0, 0.1) && floatEq(gen1.P, 0, 0.1) && floatEq(grid1.P, -200, 0.1) && floatEq(loadLv.P, -200, 0.1)) {
		t.Log("Test 3: Grid forming - one ess, one gen, grid and load")
		t.Logf("Got ESS output %.1fkW, Gen output %.1fkW, grid output %.1fkW, load output %.1fkW", ess1.P, gen1.P, grid1.P, loadLv.P)
		t.Log("Expected 0kW, 0kW, -200kW, -200kW")
		t.Fail()
	}
	if !(floatEq(loadLv.V, 479.6, 0.1) && floatEq(loadLv.F, 60, 0.01)) { // 479.6V instead of 480V because of turns ratio (12470/26 = 479.6)
		t.Log("Test 3: Grid forming - one ess, one gen, grid and load")
		t.Logf("Got load voltage %.1fV, load frequency %.3f", loadLv.V, loadLv.F)
		t.Log("Expected 479.6V, 60Hz")
		t.Fail()
	}

	// Put generator in grid following mode, give it a diferent command
	gen1.GridFollowingCmd = true
	gen1.Pcmd = 300
	gen1.Qcmd = -100
	// First go is to get the output set
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	// Second iteration is to let droop take effect
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	if !(floatEq(ess1.P, 0, 0.1) && floatEq(gen1.P, 300, 0.1) && floatEq(grid1.P, 100, 0.1) && floatEq(loadLv.P, -200, 0.1)) {
		t.Log("Test 4: Grid forming - one ess, one gen in grid following, grid and load")
		t.Logf("Got ESS output %.1fkW, Gen output %.1fkW, grid output %.1fkW, load output %.1fkW", ess1.P, gen1.P, grid1.P, loadLv.P)
		t.Log("Expected 0kW, 0kW, -200kW, -200kW")
		t.Fail()
	}
	if !(floatEq(loadLv.V, 479.6, 0.1) && floatEq(loadLv.F, 60, 0.01)) {
		t.Log("Test 4: Grid forming - one ess, one gen in grid following, grid and load")
		t.Logf("Got load voltage %.1fV, load frequency %.3f", loadLv.V, loadLv.F)
		t.Log("Expected 479.6V, 60Hz")
		t.Fail()
	}
	// Disconnect the grid again, still cool?
	feedSite.Opencmd = true
	// First go is to get the output set
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	// Second iteration is to let droop take effect
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	if !(floatEq(ess1.P, -100, 0.1) && floatEq(gen1.P, 300, 0.1) && floatEq(grid1.P, 0, 0.1) && floatEq(loadLv.P, -200, 0.1)) {
		t.Log("Test 5: Grid forming - one ess, one gen in grid following and load")
		t.Logf("Got ESS output %.1fkW, Gen output %.1fkW, grid output %.1fkW, load output %.1fkW", ess1.P, gen1.P, grid1.P, loadLv.P)
		t.Log("Expected -100kW, 300kW, 0kW, -200kW")
		t.Fail()
	}
	if !(floatEq(loadLv.V, 408, 0.1) && floatEq(loadLv.F, 60.3, 0.01)) {
		t.Log("Test 5: Grid forming - one ess, one gen in grid following and load")
		t.Logf("Got load voltage %.1fV, load frequency %.3f", loadLv.V, loadLv.F)
		t.Log("Expected 408V, 60.3Hz")
		t.Fail()
	}

	// Turn off grid forming, make sure V=0
	ess1.Offcmd = true
	gen1.Offcmd = true
	// I think this should only take one iteration
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	if !(floatEq(ess1.P, 0, 0.1) && floatEq(gen1.P, 0, 0.1) && floatEq(loadLv.P, 0, 0.1)) {
		t.Log("Test 6: Grid forming - grid disconnected, all grid forming assets off")
		t.Logf("Got ESS output %.1fkW, Gen output %.1fkW, load output %.1fkW", ess1.P, gen1.P, loadLv.P)
		t.Log("Expected 0kW, 0kW, 0kW")
		t.Fail()
	}
	if !(floatEq(loadLv.V, 0, 0.1) && floatEq(loadLv.F, 0, 0.01)) {
		t.Log("Test 6: Grid forming - grid disconnected, all grid forming assets off")
		t.Logf("Got load voltage %.1fV, load frequency %.3f", loadLv.V, loadLv.F)
		t.Log("Expected 0V, 0Hz")
		t.Fail()
	}

	// Restart the ESS as grid forming, should get same results as Test 1
	// Test load only supplied by ESS on 480V bus
	// Grid is disconnected
	ess1.Oncmd = true
	loadLv.Oncmd = true
	// First go is to get the output set
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	// Second iteration is to let droop take effect
	discoverTree(testRoot, testRoot.asset.Term(), 600)
	calculateTree(testRoot, testRoot.asset.Term(), 600)
	updateTree(testRoot, testRoot.asset.Term(), 600)
	if !(ess1.P == 200 && grid1.P == 0 && loadLv.P == -200) {
		t.Log("Test 7: Grid forming - one ess, load only")
		t.Logf("Got ESS output %.1fkW, grid output %.1fkW, load output %.1fkW", ess1.P, grid1.P, loadLv.P)
		t.Log("Expected 200kW, 0kW, -200kW")
		t.Fail()
	}
	if !(floatEq(loadLv.V, 456, 0.1) && floatEq(loadLv.F, 59.4, 0.01)) {
		t.Log("Test 7: Grid forming - one ess, load only")
		t.Logf("Got load voltage %.1fV, load frequency %.3f", loadLv.V, loadLv.F)
		t.Log("Expected 456V, 59.4Hz")
		t.Fail()
	}
}
