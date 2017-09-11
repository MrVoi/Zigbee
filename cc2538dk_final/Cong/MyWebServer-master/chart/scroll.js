	
window.onload = function () {
		var dps = []; // dataPoints
		var chart = new CanvasJS.Chart("scrollChart",{
			height: 230,
			title :{
				text: "Scrolling chart"
			},			
			data: [{
				type: "line",
				dataPoints: dps 
			}]
			
		});
		var xVal = 0;
		var yVal = 100;	
		var updateInterval = 100;
		var dataLength = 100; // number of dataPoints visible at any point
		
		//var updateChart = function () {
			
			$.getJSON('/chart/chart.php',
					{value:' Lux'},
					function(data){ 
						$.each(data, function(key, value){ 
							dps.push({
								x: key,
								y: value
							});
						
						});
						alert(dps);
					});
					
						
			
			//if (dps.length > dataLength)
			//{
			//	dps.shift();				
			//}
			
			chart.render();		

		//};

		// generates first set of dataPoints
		//updateChart(); 

		// update chart after specified time. 
		//setInterval(function(){updateChart()}, updateInterval); 
	}

	
	