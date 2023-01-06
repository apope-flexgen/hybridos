package main

import (
	"github.com/therecipe/qt/widgets"
	"github.com/therecipe/qt/charts"
	"github.com/therecipe/qt/core"
	"os"
)

func main() {
	// Create application
	app := widgets.NewQApplication(len(os.Args), os.Args)

	// Create main window
	window := widgets.NewQMainWindow(nil, 0)
	window.SetWindowTitle("Chart Setup Example")
	window.SetMinimumSize2(500, 500)
	series:= charts.NewQLineSeries(nil)
	series.Append(0, 6);
	series.Append(2, 4);
	series.Append(3, 8);
	series.Append(7, 4);
	series.Append(10, 6);
	chart := charts.NewQChart(nil, core.Qt__Widget)
	//chart := widgets.NewQChart()
	chart.AddSeries(series) 
	chart.CreateDefaultAxes()
	chart.SetTitle("demo chart")
    chart.Legend().Hide()

	chartview := charts.NewQChartView2(chart, nil)

	// chart.Series.Append(2, 4);
	// chart.Series.Append(3, 8);
	// chart.Series.Append(7, 4);
	// chart.Series.Append(10, 6);
	
	// Create main layout
	layout := widgets.NewQVBoxLayout()

	// Create main widget and set the layout
	mainWidget := widgets.NewQWidget(nil, 0)
	mainWidget.SetLayout(layout)

	// Create a line edit and add it to the layout
	input := widgets.NewQLineEdit(nil)
	input.SetPlaceholderText("1. write something")
	layout.AddWidget(input, 0, 0)

	// Create a button and add it to the layout
	button := widgets.NewQPushButton2("2. click me", nil)
	layout.AddWidget(button, 0, 0)

	window2 := widgets.NewQMainWindow(nil, 0)
	window2.SetWindowTitle("Chart Example")
	window2.SetMinimumSize2(500, 500)
	window2.SetCentralWidget(chartview)
	i := 11.0
	j := 7.0
	//layout.AddWidget(chartview, 0, 0)
	// Connect event for button
	button.ConnectClicked(func(checked bool) {
		//widgets.QMessageBox_Information(nil, "OK", input.Text(),
		//widgets.QMessageBox__Ok, widgets.QMessageBox__Ok)
		// Show the window
		i += 1.0
		j -= 0.5
		series.Append(i, j)
		chart.RemoveSeries(series)
		chart.AddSeries(series) 

		window2.Show()

	})

	// Set main widget as the central widget of the window
	window.SetCentralWidget(mainWidget)

	// Show the window
	window.Show()

	// Execute app
	app.Exec()
}