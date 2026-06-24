
// State
let originalText = '';
let compressedBase64 = '';
let decompressedText = '';
let huffmanCodes = {};
let treeJSON = null;

// DOM elements
const textInput = document.getElementById('text-input'); 
const charCounter = document.getElementById('char-counter'); 
const dropzone = document.getElementById('dropzone'); 
const fileInput = document.getElementById('file-input'); 

const compressBtn = document.getElementById('compress-btn'); 
const decompressBtn = document.getElementById('decompress-btn');

const statOrigSize = document.getElementById('stat-orig-size');
const statCompSize = document.getElementById('stat-comp-size');
const statRatio = document.getElementById('stat-ratio');
const statReduction = document.getElementById('stat-reduction');
const statTime = document.getElementById('stat-time');
const statIntegrity = document.getElementById('stat-integrity');
const statIntegrityBox = document.getElementById('stat-integrity-box');
const statEffectiveness = document.getElementById('stat-effectiveness');

const downloadCompBtn = document.getElementById('download-comp-btn');
const downloadDecompBtn = document.getElementById('download-decomp-btn');
const copyBitsBtn = document.getElementById('copy-bits-btn');

const codesTable = document.getElementById('codes-table').getElementsByTagName('tbody')[0];
const bitstreamViewer = document.getElementById('bitstream-viewer');
const toast = document.getElementById('toast');

// Backend configuration
const API_URL = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1' || window.location.hostname === ''
    ? window.location.origin
    : 'https://huffman-engine-backend.onrender.com';

function initTheme() {
    document.documentElement.setAttribute('data-theme', 'dark');
}

// Toast alerts

function showToast(message, type = 'success') { 
    toast.textContent = message;
    toast.className = `toast ${type}`;
    toast.classList.remove('hidden');
    setTimeout(() => {
        toast.classList.add('hidden');
    }, 4000);
}


function setIntegrityPending() {
    if (statIntegrity && statIntegrityBox) {
        statIntegrity.textContent = 'PENDING';
        statIntegrityBox.className = 'stat-box';
    }
}

function updateCharCounter() {
    const count = textInput.value.length;
    charCounter.textContent = `${count.toLocaleString()} character${count !== 1 ? 's' : ''}`;
    
    // Enable/disable compress button based on input presence
    if (count > 0) {
        compressBtn.removeAttribute('disabled');
        // Clear decompress state when user types new text
        decompressBtn.setAttribute('disabled', 'true');
    }
    setIntegrityPending();
}

textInput.addEventListener('input', updateCharCounter);

// File Upload & Drag-and-Drop Management

dropzone.addEventListener('click', () => fileInput.click());

dropzone.addEventListener('dragover', (e) => {
    e.preventDefault();
    dropzone.classList.add('dragover');
});

dropzone.addEventListener('dragleave', () => {
    dropzone.classList.remove('dragover');
});

dropzone.addEventListener('drop', (e) => {
    e.preventDefault();
    dropzone.classList.remove('dragover');
    if (e.dataTransfer.files.length > 0) {
        handleFile(e.dataTransfer.files[0]);
    }
});

fileInput.addEventListener('change', (e) => {
    if (e.target.files.length > 0) {
        handleFile(e.target.files[0]);
    }
});

function handleFile(file) {
    const reader = new FileReader();
    const isHuff = file.name.endsWith('.huff');

    if (isHuff) {
        reader.onload = function(e) {
            const dataUri = e.target.result;
            compressedBase64 = dataUri.split(',')[1];
            
            textInput.value = '';
            updateCharCounter();
            
            // Reset stats UI
            statOrigSize.textContent = '-';
            statCompSize.textContent = `${file.size.toLocaleString()} bytes`;
            statRatio.textContent = '-';
            statReduction.textContent = '-';
            statTime.textContent = '-';
            statIntegrity.textContent = 'PENDING';
            statIntegrityBox.className = 'stat-box';
            statEffectiveness.classList.add('hidden');
 
            // Update button states
            compressBtn.setAttribute('disabled', 'true');
            decompressBtn.removeAttribute('disabled');
            downloadCompBtn.setAttribute('disabled', 'true');
            downloadDecompBtn.setAttribute('disabled', 'true');
            copyBitsBtn.setAttribute('disabled', 'true');
 
            showToast(`Loaded compressed file: ${file.name}. Click 'Decompress' to restore text.`, 'success');
        };
        reader.readAsDataURL(file);
    } else {
        reader.onload = function(e) {
            textInput.value = e.target.result;
            updateCharCounter();
            
            compressBtn.removeAttribute('disabled');
            decompressBtn.setAttribute('disabled', 'true');
            
            showToast(`Loaded text file: ${file.name}. Ready for compression.`, 'success');
        };
        reader.readAsText(file);
    }
}

// UI helpers

function formatChar(ascii) {
    const code = parseInt(ascii, 10);
    if (code === 32) return 'Space';
    if (code === 10) return '\\n (Newline)';
    if (code === 13) return '\\r (Return)';
    if (code === 9) return '\\t (Tab)';
    if (code >= 32 && code < 127) return `'${String.fromCharCode(code)}'`;
    return `Char ${code}`;
}

// Compress handler
compressBtn.addEventListener('click', async () => {
    const text = textInput.value;
    if (!text) return;

    compressBtn.querySelector('.loader').classList.remove('hidden');
    compressBtn.setAttribute('disabled', 'true');
    setIntegrityPending();

    try {
        const response = await fetch(`${API_URL}/api/compress`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ text })
        });

        const data = await response.json();
        if (data.error) {
            throw new Error(data.error);
        }

        originalText = text;
        compressedBase64 = data.compressedBase64;
        huffmanCodes = data.codes;
        treeJSON = data.treeJSON;

        // Render Stats
        statOrigSize.textContent = `${data.originalSize.toLocaleString()} bytes`;
        statCompSize.textContent = `${data.compressedSize.toLocaleString()} bytes`;
        statRatio.textContent = `${data.ratio.toFixed(2)}x`;
        statReduction.textContent = `${data.reduction.toFixed(2)}%`;
        statTime.textContent = `${data.timeMs.toFixed(2)} ms`;
        statIntegrity.textContent = 'PENDING';
        statIntegrityBox.className = 'stat-box';

        if (data.effectiveness) {
            statEffectiveness.textContent = data.effectiveness;
            statEffectiveness.className = 'effectiveness-badge';
            
            const eff = data.effectiveness.toLowerCase();
            if (eff.includes('excellent')) {
                statEffectiveness.classList.add('excellent');
            } else if (eff.includes('good')) {
                statEffectiveness.classList.add('good');
            } else if (eff.includes('neutral')) {
                statEffectiveness.classList.add('neutral');
            } else if (eff.includes('not beneficial') || eff.includes('not-beneficial')) {
                statEffectiveness.classList.add('not-beneficial');
            }
        } else {
            statEffectiveness.classList.add('hidden');
        }

        // Populate code lookup table
        codesTable.innerHTML = '';
        const sortedCodes = Object.entries(data.codes).sort((a, b) => a[1].length - b[1].length);
        
        if (sortedCodes.length === 0) {
            codesTable.innerHTML = `<tr class="placeholder-row"><td colspan="4">No codes generated</td></tr>`;
        } else {
            sortedCodes.forEach(([ascii, code]) => {
                const row = codesTable.insertRow();
                row.insertCell(0).textContent = formatChar(ascii);
                row.insertCell(1).textContent = ascii;
                row.insertCell(2).innerHTML = `<span style="font-family:'Fira Code',monospace;font-weight:600;color:var(--secondary)">${code}</span>`;
                row.insertCell(3).textContent = code.length;
            });
        }

        // Render bitstream
        bitstreamViewer.innerHTML = `<div style="color:var(--secondary);word-wrap:break-word;">${data.bitString || 'Empty bitstream'}</div>`;

        // Enable actions
        decompressBtn.removeAttribute('disabled');
        downloadCompBtn.removeAttribute('disabled');
        copyBitsBtn.removeAttribute('disabled');

        showToast('Compression completed successfully!', 'success');

    } catch (error) {
        showToast(error.message, 'error');
    } finally {
        compressBtn.querySelector('.loader').classList.add('hidden');
        compressBtn.removeAttribute('disabled');
    }
});

// Decompress handler
decompressBtn.addEventListener('click', async () => {
    if (!compressedBase64) return;

    decompressBtn.querySelector('.loader').classList.remove('hidden');
    decompressBtn.setAttribute('disabled', 'true');
    setIntegrityPending();

    try {
        const response = await fetch(`${API_URL}/api/decompress`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ compressedBase64 })
        });

        const data = await response.json();
        if (data.error) {
            throw new Error(data.error);
        }

        decompressedText = data.decompressedText;

        // Show decompressed text if text area is empty
        if (!textInput.value) {
            textInput.value = decompressedText;
            updateCharCounter();
        }

        // Verify Data Integrity
        const matchesOriginal = (originalText === decompressedText) || (textInput.value === decompressedText);
        if (matchesOriginal) {
            statIntegrity.textContent = 'PASSED';
            statIntegrityBox.className = 'stat-box passed';
            showToast('Data Integrity Check: PASSED. 100% Lossless Recovery!', 'success');
        } else {
            statIntegrity.textContent = 'FAILED';
            statIntegrityBox.className = 'stat-box failed';
            showToast('Data Integrity Check: FAILED. Recovered content mismatch.', 'error');
        }

        // Display decompression timing
        statTime.textContent = `${data.timeMs.toFixed(2)} ms (decomp)`;
        downloadDecompBtn.removeAttribute('disabled');

    } catch (error) {
        showToast(error.message, 'error');
    } finally {
        decompressBtn.querySelector('.loader').classList.add('hidden');
        decompressBtn.removeAttribute('disabled');
    }
});

// Actions: Download and Utility Events

downloadCompBtn.addEventListener('click', () => {
    if (!compressedBase64) return;
    const blob = base64ToBlob(compressedBase64, 'application/octet-stream');
    triggerDownload(blob, 'compressed.huff');
});

downloadDecompBtn.addEventListener('click', () => {
    if (!decompressedText) return;
    const blob = new Blob([decompressedText], { type: 'text/plain;charset=utf-8' });
    triggerDownload(blob, 'decompressed.txt');
});

copyBitsBtn.addEventListener('click', () => {
    const bitText = bitstreamViewer.textContent.trim();
    if (!bitText || bitText.includes('Compressed bit sequence')) return;

    navigator.clipboard.writeText(bitText)
        .then(() => showToast('Bitstream copied to clipboard!', 'success'))
        .catch(() => showToast('Failed to copy to clipboard', 'error'));
});

function base64ToBlob(base64, mimeType) {
    const byteCharacters = atob(base64);
    const byteNumbers = new Array(byteCharacters.length);
    for (let i = 0; i < byteCharacters.length; i++) {
        byteNumbers[i] = byteCharacters.charCodeAt(i);
    }
    const byteArray = new Uint8Array(byteNumbers);
    return new Blob([byteArray], { type: mimeType });
}

function triggerDownload(blob, filename) {
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}


// Initialize on load
initTheme();
updateCharCounter();
