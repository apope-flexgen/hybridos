const express = require('express');
const bodyParser = require('body-parser');
const { exec } = require('child_process');

const app = express();
app.use(bodyParser.json());

app.post('/run-command', (req, res) => {
    const { dest, ip, cmd, args, jobid } = req.body;

    if (dest && dest !== process.env.SERVER_IP) {
        // Forward the request (This is a simplified example, in production you need to handle this more carefully)
        exec(`curl -X POST -H "Content-Type: application/json" --data '${JSON.stringify(req.body)}' http://${dest}:5000/run-command`, 
            (error, stdout, stderr) => {
                if (error) {
                    return res.status(500).json({ status: 'error', message: error.message });
                }
                res.json(JSON.parse(stdout));
            });
        return;
    }

    // Execute the command (Here, you'd want to implement the actual command execution logic)
    exec(`${cmd} ${args}`, (error, stdout, stderr) => {
        if (error) {
            return res.status(500).json({ status: 'error', message: error.message });
        }
        res.json({ status: 'success', output: stdout });
    });
});

app.post('/send-file', (req, res) => {
    const { dest, filename, content } = req.body;

    if (dest && dest !== process.env.SERVER_IP) {
        // Forward the request
        exec(`curl -X POST -H "Content-Type: application/json" --data '${JSON.stringify(req.body)}' http://${dest}:5000/send-file`, 
            (error, stdout, stderr) => {
                if (error) {
                    return res.status(500).json({ status: 'error', message: error.message });
                }
                res.json(JSON.parse(stdout));
            });
        return;
    }

    // Write content to file
    fs.writeFile(filename, content, (err) => {
        if (err) {
            return res.status(500).json({ status: 'error', message: err.message });
        }
        res.json({ status: 'success', message: 'File written successfully' });
    });
});

app.post('/get-file', (req, res) => {
    const { dest, filename } = req.body;

    if (dest && dest !== process.env.SERVER_IP) {
        // Forward the request
        exec(`curl -X POST -H "Content-Type: application/json" --data '${JSON.stringify(req.body)}' http://${dest}:5000/get-file`, 
            (error, stdout, stderr) => {
                if (error) {
                    return res.status(500).json({ status: 'error', message: error.message });
                }
                res.json(JSON.parse(stdout));
            });
        return;
    }

    // Read content from file
    fs.readFile(filename, 'utf8', (err, content) => {
        if (err) {
            return res.status(500).json({ status: 'error', message: err.message });
        }
        res.json({ status: 'success', content: content });
    });
});

app.listen(5000, () => {
    console.log('Server is running on port 5000');
});