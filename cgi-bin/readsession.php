<?php
	// Start the session
	session_start();
?>
<!DOCTYPE html>
<html>
<head>
	<title>Read Session Variable</title>
</head>
<body>
	<h1>Read Session Variable</h1>
	<?php
		if (isset($_SESSION["example"])) {
			echo "<p>Session variable 'example' is set to: " . $_SESSION["example"] . "</p>";
		} else {
			echo "<p>Session variable 'example' is not set.</p>";
		}
	?>
</body>
</html>