const express = require('express');
const cors = require('cors');
const multer = require('multer');
const { execFile } = require('child_process');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const app = express();
const PORT = process.env.PORT || 3000;

app.use(cors());
app.use(express.json({ limit: '50mb' }));

// Request Logging Middleware
app.use((req, res, next) => {
    console.log(`[REQUEST] ${req.method} ${req.url}`);
    next();
});

app.use(express.static(path.join(__dirname, 'public')));

// Configure Multer for temp file uploads
const upload = multer({ dest: 'temp/' });

// Ensure temp directory exists
const tempDir = path.join(__dirname, 'temp');
if (!fs.existsSync(tempDir)) {
    fs.mkdirSync(tempDir);
}

// Run the C++ engine with args and return parsed JSON
function runCompressor(args) {
    return new Promise((resolve, reject) => {
        const binaryPath = path.join(__dirname, 'huffman_app');
        console.log(`[EXEC] Running: ${binaryPath} ${args.join(' ')}`);
        execFile(binaryPath, args, { maxBuffer: 1024 * 1024 * 50 }, (error, stdout, stderr) => {
            if (error) {
                console.error(`[EXEC ERROR] Code execution failed: ${stderr || error.message}`);
                return reject(new Error(stderr || error.message));
            }
            try {
                const response = JSON.parse(stdout.trim());
                if (response.error) {
                    return reject(new Error(response.error));
                }
                resolve(response);
            } catch (e) {
                reject(new Error(`Failed to parse C++ engine response: ${stdout}`));
            }
        });
    });
}

// Compress API Endpoint
app.post('/api/compress', upload.single('file'), async (req, res) => {
    let inputFilePath = '';
    let outputFilePath = '';
    const uniqueId = crypto.randomBytes(8).toString('hex');

    try {
        let inputText = '';
        if (req.file) {
            inputFilePath = req.file.path;
            inputText = fs.readFileSync(inputFilePath, 'utf8');
        } else if (req.body.text !== undefined) {
            inputText = req.body.text;
            inputFilePath = path.join(tempDir, `input_${uniqueId}.txt`);
            fs.writeFileSync(inputFilePath, inputText, 'utf8');
        } else {
            return res.status(400).json({ error: 'No text or file provided.' });
        }

        outputFilePath = path.join(tempDir, `output_${uniqueId}.huff`);

        // Execute C++ Compression
        const result = await runCompressor(['-api', '-c', inputFilePath, outputFilePath]);

        // Read the resulting compressed .huff file and encode as Base64
        let compressedBase64 = '';
        if (fs.existsSync(outputFilePath)) {
            const compressedBuffer = fs.readFileSync(outputFilePath);
            compressedBase64 = compressedBuffer.toString('base64');
        }

        res.json({
            ...result,
            originalText: inputText,
            compressedBase64
        });

    } catch (err) {
        res.status(500).json({ error: err.message });
    } finally {
        // Clean up temporary files
        if (inputFilePath && fs.existsSync(inputFilePath) && !req.file) {
            fs.unlinkSync(inputFilePath);
        }
        if (req.file && fs.existsSync(req.file.path)) {
            fs.unlinkSync(req.file.path);
        }
        if (outputFilePath && fs.existsSync(outputFilePath)) {
            fs.unlinkSync(outputFilePath);
        }
    }
});

// Decompress API Endpoint
app.post('/api/decompress', upload.single('file'), async (req, res) => {
    let inputFilePath = '';
    let outputFilePath = '';
    const uniqueId = crypto.randomBytes(8).toString('hex');

    try {
        if (req.file) {
            inputFilePath = req.file.path;
        } else if (req.body.compressedBase64) {
            const buffer = Buffer.from(req.body.compressedBase64, 'base64');
            inputFilePath = path.join(tempDir, `input_${uniqueId}.huff`);
            fs.writeFileSync(inputFilePath, buffer);
        } else {
            return res.status(400).json({ error: 'No compressed file or data provided.' });
        }

        outputFilePath = path.join(tempDir, `output_${uniqueId}.txt`);

        // Execute C++ Decompression
        const result = await runCompressor(['-api', '-d', inputFilePath, outputFilePath]);

        res.json(result);

    } catch (err) {
        res.status(500).json({ error: err.message });
    } finally {
        // Clean up temporary files
        if (inputFilePath && fs.existsSync(inputFilePath)) {
            fs.unlinkSync(inputFilePath);
        }
        if (outputFilePath && fs.existsSync(outputFilePath)) {
            fs.unlinkSync(outputFilePath);
        }
    }
});

app.listen(PORT, () => {
    console.log(`[SERVER] REST API Wrapper running on http://localhost:${PORT}`);
});
