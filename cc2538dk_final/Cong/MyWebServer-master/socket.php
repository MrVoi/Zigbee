<?php
$sql="";
$action="";
$servername='localhost';
$username='root';
$password='11235813';
$dbname='nodedata';
if ($_SERVER["REQUEST_METHOD"] == "POST")
{
	if(!empty($_POST['action'])){
		$action=$_POST['action'];
		if(($action=='start')||($action=='stop'))
		{
			$sock = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
			$len =  strlen($action);
			socket_sendto($sock, $action, $len, 0, '127.0.0.1', 12345);
			socket_close($sock);
			echo $action;
		}
		elseif($action=='delete')
		{
			$conn = mysqli_connect($servername, $username, $password, $dbname);
			if (!$conn) {
				die("Connection failed: " . $conn->connect_error);
			}
			$sql = "truncate node";
			mysqli_query($conn, $sql);
			$conn->close();
		}
	}
}
?>