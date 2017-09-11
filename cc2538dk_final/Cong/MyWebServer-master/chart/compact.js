
window.onload = function () {
	
	var dps = []; // dataPoints
	var minRange = 0;
	var maxRange = 0;
	var slideBy = 0;
	var zoomed = false;
	var axisXOptions = {viewportMinimum: null,   viewportMaximum: null};



	var chart = new CanvasJS.Chart("compactChart",{
	  zoomEnabled: true,
	  title :{
		text: "Compact Chart"
	  },
	  height:230,
	  rangeChanged: function(e){
		if (e.trigger === "reset") {
				chart.options.axisX.viewportMinimum = chart.options.axisX.viewportMaximum = null;
							zoomed = false;
			} 
		else {
			minRange = e.axisX.viewportMinimum;
				maxRange = e.axisX.viewportMaximum;
			zoomed = true;
			}
		slideBy = 0;
	  },
	  
	  axisX: axisXOptions,
	  data: [{
		type: "line",  
		markerType: "none",
		dataPoints: dps
	  }]
	});



	var xVal = 0;
	var yVal = 100;	
	var updateInterval = 1000;
	var dataLength = 100; // number of dataPoints visible at any point
	var queryData=[];

	var updateChart = function (count) {
		 count = count || 1;

	  // count is number of times loop runs to generate random dataPoints.

	  for (var j = 0; j < count; j++) {	
		dps.push({
		  x: xVal,
		  y: 
		});
		xVal++;
		if(zoomed)
			{   
				axisXOptions.viewportMinimum =  minRange + slideBy;
					axisXOptions.viewportMaximum = maxRange + slideBy;
				slideBy += dps[dps.length-1].x-dps[dps.length-2].x;
			}
	  };
	  chart.render();		
	};

	// generates first set of dataPoints
	updateChart(dataLength); 

	// update chart after specified time. 
	setInterval(function(){updateChart()}, updateInterval);
}