<!DOCTYPE html>
<html>
<head>
	<title>PHP Info</title>
</head>
<body>
	<h1>PHP Info</h1>
	<?php
		// Get all files in the current directory
		$files = glob('*');

		// Display each file with a delete button
		foreach ($files as $file) {
			if (is_file($file)) {
				$functionName = 'sendDeleteRequest' . md5($file);
				echo "
				<p>$file</p>
				<button onclick=\"$functionName()\">Send DELETE Request</button>

				<script>
				function $functionName() {
					fetch('$file', {
						method: 'DELETE',
					})
					.then(response => response.text())
					.then(data => {
						console.log(data);
						window.location.reload(); // Refresh the page
					})
					.catch((error) => console.error('Error:', error));
				}
				</script>";
			}
		}
	?>
</body>
</html>