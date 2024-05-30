<!DOCTYPE html>
<html>
<head>
	<title>Beautiful Page</title>
	<style>
		body {
			background-color: #f0f0f0;
			font-family: Arial, sans-serif;
			text-align: center;
			padding-top: 50px;
		}
		h1 {
			color: #333;
		}
	</style>
</head>
<body>
	<h1>
	<?php
		date_default_timezone_set('Europe/Berlin');
		$hour = date('G');
		if ($hour < 12) {
			echo 'Good Morning!';
		} elseif ($hour < 18) {
			echo 'Good Afternoon!';
		} else {
			echo 'Good Evening!';
		}
	?>
	</h1>
	<p>Welcome to our Beautiful Page!</p>
	<p>This is a simple PHP page that prints something beautiful.</p>
</body>
</html>