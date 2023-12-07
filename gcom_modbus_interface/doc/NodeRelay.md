Certainly! Below is a Node.js version of the script you provided:

Firstly, install the required package via npm:

```bash
npm install express body-parser request
```

Then, you can create a file for your Node.js server script:

```javascript
const express = require('express');
const bodyParser = require('body-parser');
const request = require('request');
const { exec } = require('child_process');

const app = express();
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

const serverIP = process.argv[2];

if (!serverIP) {
  console.log('Usage: node <scriptname> <server_ip>');
  process.exit(1);
}

app.get('/run-command', (req, res) => {
  const { route, ip, cmd, args, jobid } = req.query;

  if (route && route !== serverIP) {
    // Forward the request
    const forwardedURL = `http://${route}:5000/run-command?${req.originalUrl.split('?')[1]}`;
    request(forwardedURL, (error, response, body) => {
      if (error) return res.json({ status: 'error', dest: route, message: 'Connection refused to destination server' });
      res.send(body);
    });
  } else {
    // Execute the command and get the result
    exec(`${cmd} ${args}`, (error, stdout, stderr) => {
      let result;
      if (error) {
        result = { status: 'error', output: stderr, message: error.toString() };
      } else {
        result = { status: 'success', output: stdout };
      }
      console.log(result);
      res.json(result);
    });
  }
});

const PORT = 5000;
app.listen(PORT, () => console.log(`Server is running on http://localhost:${PORT}`));
```

### Usage:
1. Run the script using Node with the server IP as an argument, like so:
    ```bash
    node scriptname.js <server_ip>
    ```
2. You can then send GET requests to `http://localhost:5000/run-command` with the appropriate query parameters.

### Note:
- Ensure Node.js is installed on your system.
- The script uses the `child_process` module to execute commands; make sure the commands are safe and sanitized to prevent command injection attacks.
- The script requires the `express`, `body-parser`, and `request` npm packages, so make sure to install them before running the script.