<?php

$servername='localhost';
$username='root';
$password='11235813';
$dbname='nodedata';
if ($_SERVER["REQUEST_METHOD"] == "GET")
{
	if(!empty($_GET['value']))
	{
		$value=$_GET['value'];
		$jsonArray=array();
		$conn = new mysqli($servername, $username, $password, $dbname);
		if ($conn->connect_error) {
		  die("Connection failed: " . $conn->connect_error);
		}
		$sql =  "SELECT ".$value." FROM node";
		//echo $sql;
		$result = $conn->query($sql);
		$data=array();
		if ($result->num_rows > 0) {
			while($row = $result->fetch_assoc()){
				array_push($data, $row[$value]);
			}
			$myJson=json_encode($data);
			echo $myJson;
		}
		else{
			echo 'No data';
		}
		$conn->close();

	}
}
?>