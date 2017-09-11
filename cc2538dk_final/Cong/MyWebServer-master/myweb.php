<html lang="vi">
	<head>
		<meta charset="utf-8"/>
		<title>Thanh Cong</title>
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<meta name="author" content="Thanh Cong"/>
		<link rel="stylesheet" href="./css/base.css"/>
		<link rel="stylesheet" href="./css/pure.css"/>
		<link rel="stylesheet" href="style.css"/>
		<script type="text/javascript" src="./chart/canvasjs.min.js"/></script>
		<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>

		<script type="text/javascript">
		var jvalue="Temperature";
		var scroll=true;
		function selectValue(str){
				jvalue=str;
				
			}
		$(document).ready(function(){

			$("#changeChart").click(function(e){
				e.preventDefault();
				if(scroll==true)
				{
					scroll=false
				}
				else
				{
					scroll=true;
				}
				//alert(currentChart);
				
			});
			$("#start").click(function(e){
				e.preventDefault();
				$.post("socket.php", {action: 'start'},
				//function(data,status){
					//alert("Data: " + data + "\nStatus: " + status);
				//}
				);
			});
			$("#stop").click(function(e){
				e.preventDefault();
				$.post("socket.php", {action: 'stop'},
				//function(data,status){
					//alert("Data: " + data + "\nStatus: " + status);
				//}
				);
			});
			$("#delete").click(function(e){
				e.preventDefault();
				$.post("socket.php", {action: 'delete'},
				//function(data,status){
					//alert("Data: " + data + "\nStatus: " + status);
				//}
				);
			});
		});
		window.onload = function() {
		var dt=[];
			var chart = new CanvasJS.Chart("chartContainer", {
			height: 230,
			title: {
				text: "Line Chart"
			},
			axisX: {
				interval: 10
			},
			data: [{
				type: "line",
				dataPoints: dt
			}]
		});
		chart.render();
		
			
		var doIt=function(){
			if(scroll)
			{
				while(dt.length>20)
				{
					dt.shift();
				}
			}
			chart.render();
			while(dt.length>0)
			{
				dt.pop();
			}
		};
			
		var updateChart=function(){
			
			$.getJSON('/chart/chart.php', 
				{value: jvalue},
				function(data, status){
					
					$.each(data, function(key, value){dt.push({x: parseInt(key), y: parseInt(value)/100});});
						doIt();

				}
			);

		};
		setInterval(function(){updateChart()}, 1000); 
}
		</script>

	</head>
	<body>
		<div id="header" class="container">
			<div id="logo">
				<img src="logo.png" title="Logo" alt="Bach Khoa"/>
			</div>
			<div id="hello">
				<p>TRƯỜNG ĐẠI HỌC BÁCH KHOA THÀNH PHỐ HỒ CHÍ MINH<br/>
				KHOA ĐIỆN-ĐIỆN TỬ<br/>
				BỘ MÔN VIỄN THÔNG
				</p>
			
			</div>
		</div>
		<div id="menu">
			<ul>
				<li><a href='#'>Trang chủ</a></li>
				<li><a href='#'>Tin Tức</a></li>
				<li><a href='#'>Hỏi Đáp</a></li>
				<li><a href='#'>Liên Hệ</a></li>
				<li><a href='#'>Tham Khảo</a></li>
				<li><a href='#'>Nguồn</a></li>
			</ul>
		</div>
		
		<div id="login-form">
			<form class="pure-form">
				<fieldset>
					<input type="email" placeholder="Email">
					<input type="password" placeholder="Password">

					<label for="remember">
						<input id="remember" type="checkbox"> Remember me
					</label>

					<button type="submit" class="pure-button pure-button-primary">Sign in</button>
				</fieldset>
			</form>
			<div class="clear"></div>
		</div>
		<div id='client'>
			<fieldset>
				<legend>Client information</legend>
					<h2 id="client-info">Đồ án môn học 2</h2>
			</fieldset>
		</div>			
			
		<div id='node' style="padding: 10px 0 0 0;">
			<fieldset style="height: 300px; overflow: auto;">
				<legend>Node information</legend>
				<div id='Cmd'>
					<form class="pure-form" method="post">
						<p id="buttonStatus">Button status</p>
						<button id='start' name="start" class='pure-button pure-button-primary' >Start</button>
						<button id='stop' name="stop" class="pure-button pure-button-primary" >Stop</button>
						<button id='delete' name="delete" class="pure-button pure-button-primary" >Delete data</button>
						<button id='changeChart' name="chart" class="pure-button pure-button-primary" >Change Chart</button>
					</form>
				</div>
				<div id='chart' style="width: 80%">
					<!--<script type="text/javascript" src="./chart/scroll.js"/></script>-->
					<div id='chartContainer'></div>
				</div>
				<div style="padding: 20px 0 0 82%">
				<form class="pure-form">
					<select name="select" onchange="selectValue(this.value)">
						<option value="temperature">Temperature</option>
						<option value="humidity">Humidity</option>
						<option value="lux">Lux</option>
					</select>
				</form>
				</div>
			</fieldset>
		</div>
	<!--When a button is click, the code goes here-->

	</body>
<html>
		
		
		
