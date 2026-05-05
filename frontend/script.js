document.addEventListener('DOMContentLoaded', () => {
    const findChordBtn = document.getElementById('findChordBtn');
    const chordInput = document.getElementById('chordInput');
    const errorMsg = document.getElementById('errorMsg');
    const resultsSection = document.getElementById('resultsSection');
    const shapesContainer = document.getElementById('shapesContainer');

    // Get tuning selects
    const tuningSelects = [
        document.getElementById('string6'),
        document.getElementById('string5'),
        document.getElementById('string4'),
        document.getElementById('string3'),
        document.getElementById('string2'),
        document.getElementById('string1')
    ];

    findChordBtn.addEventListener('click', async () => {
        const chordName = chordInput.value.trim();
        
        if (!chordName) {
            errorMsg.textContent = 'Please enter a chord notation (e.g. Cmaj7)';
            chordInput.focus();
            return;
        }
        
        errorMsg.textContent = '';
        
        // Gather current tuning from the UI
        // We read from 6 to 1 because 6 is low E, 1 is high E.
        // We'll pass them in order 6 to 1 to the backend.
        const tuning = tuningSelects.map(select => select.value);

        const payload = {
            chordName: chordName,
            tuning: tuning
        };

        try {
            findChordBtn.disabled = true;
            findChordBtn.innerHTML = '<span>Searching...</span>';

            // Attempt to fetch from the C++ backend
            let data;
            try {
                const response = await fetch('http://localhost:8080/api/chord', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(payload)
                });
                
                if (!response.ok) {
                    throw new Error(`Server returned ${response.status}`);
                }
                
                data = await response.json();
            } catch (err) {
                console.warn("Backend not running or unreachable, using mock data for demonstration.");
                // Provide mock data if backend isn't ready
                data = getMockData(chordName, tuning);
                
                // Simulate network delay
                await new Promise(resolve => setTimeout(resolve, 800));
            }

            renderShapes(data.shapes, tuning);
            
            resultsSection.classList.remove('hidden');
            resultsSection.scrollIntoView({ behavior: 'smooth', block: 'start' });

        } catch (error) {
            console.error('Error fetching chord:', error);
            errorMsg.textContent = 'Failed to retrieve chord shapes. See console for details.';
        } finally {
            findChordBtn.disabled = false;
            findChordBtn.innerHTML = `
                <span>Find Shapes</span>
                <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="5" y1="12" x2="19" y2="12"></line><polyline points="12 5 19 12 12 19"></polyline></svg>
            `;
        }
    });

    // Handle Enter key
    chordInput.addEventListener('keypress', (e) => {
        if (e.key === 'Enter') {
            findChordBtn.click();
        }
    });

    function renderShapes(shapes, tuning) {
        shapesContainer.innerHTML = '';

        if (!shapes || shapes.length === 0) {
            shapesContainer.innerHTML = '<p class="text-secondary">No shapes found for this chord.</p>';
            return;
        }

        shapes.forEach(shape => {
            const card = document.createElement('div');
            card.className = 'shape-card';
            
            const nameEl = document.createElement('h3');
            nameEl.className = 'shape-name';
            nameEl.textContent = shape.name || 'Chord Shape';
            card.appendChild(nameEl);

            const diagramWrapper = document.createElement('div');
            diagramWrapper.className = 'chord-diagram';
            
            // Calculate min fret for the diagram
            let minFret = 100;
            let maxFret = 0;
            shape.frets.forEach(f => {
                if (f !== null && f > 0) {
                    if (f < minFret) minFret = f;
                    if (f > maxFret) maxFret = f;
                }
            });
            
            if (minFret === 100) minFret = 1; // Open strings or muted
            
            // Ensure at least 5 frets
            let startFret = minFret > 2 ? minFret - 1 : 1; 
            
            const gridContainer = document.createElement('div');
            gridContainer.className = 'diagram-grid-container';
            
            if (startFret > 1) {
                const fretLabel = document.createElement('span');
                fretLabel.className = 'diagram-fret-label';
                fretLabel.textContent = startFret + 'fr';
                gridContainer.appendChild(fretLabel);
            }
            
            const gridAndStatus = document.createElement('div');
            gridAndStatus.className = 'diagram-grid-and-status';
            
            const grid = document.createElement('div');
            grid.className = 'diagram-grid';
            if (startFret === 1) {
                grid.classList.add('has-nut');
            }
            
            // Check for barre chord on minFret
            const minFretStrings = [];
            shape.frets.forEach((f, idx) => {
                if (f === minFret) minFretStrings.push(idx);
            });
            
            const isBarre = minFretStrings.length > 1;
            const barreMinIdx = isBarre ? Math.min(...minFretStrings) : -1;
            const barreMaxIdx = isBarre ? Math.max(...minFretStrings) : -1;
            
            // Find the lowest note (first non-muted string) to be the tonic
            let lowestNoteIndex = -1;
            for (let i = 0; i < shape.frets.length; i++) {
                if (shape.frets[i] !== null && shape.frets[i] !== 'x' && shape.frets[i] !== -1) {
                    lowestNoteIndex = i;
                    break;
                }
            }
            
            if (isBarre) {
                const barre = document.createElement('div');
                barre.className = 'diagram-barre';
                barre.style.left = `calc(${barreMinIdx * 20}% - 8px)`;
                barre.style.width = `calc(${(barreMaxIdx - barreMinIdx) * 20}% + 16px)`;
                
                const relativeFret = minFret - startFret;
                barre.style.top = (relativeFret * 20 + 10) + '%';
                
                grid.appendChild(barre);
            }
            
            // Add dots
            shape.frets.forEach((fret, index) => {
                if (fret !== null && fret > 0) {
                    // If it's part of a barre, skip rendering the individual dot
                    if (isBarre && fret === minFret) {
                        return;
                    }
                    
                    const dot = document.createElement('div');
                    dot.className = 'diagram-dot';
                    
                    if (index === lowestNoteIndex && !isBarre) {
                        dot.classList.add('tonic-dot');
                    }
                    
                    dot.style.left = (index * 20) + '%';
                    
                    const relativeFret = fret - startFret;
                    dot.style.top = (relativeFret * 20 + 10) + '%';
                    
                    grid.appendChild(dot);
                }
            });
            
            gridAndStatus.appendChild(grid);
            
            // Status row at bottom (circles and Xs)
            const statusRow = document.createElement('div');
            statusRow.className = 'diagram-status-row';
            
            shape.frets.forEach((fret, index) => {
                const status = document.createElement('div');
                status.className = 'diagram-status';
                status.style.left = (index * 20) + '%';
                
                if (fret === null || fret === 'x' || fret === -1) {
                    status.textContent = '✕'; // geometric cross instead of letter X
                    status.classList.add('muted');
                } else if (fret === 0) {
                    status.textContent = '○'; // geometric hollow circle
                    status.classList.add('open');
                } else {
                    status.textContent = '●'; // Played/strummed note
                    status.classList.add('played');
                }
                statusRow.appendChild(status);
            });
            
            gridAndStatus.appendChild(statusRow);
            gridContainer.appendChild(gridAndStatus);
            diagramWrapper.appendChild(gridContainer);
            
            card.appendChild(diagramWrapper);
            shapesContainer.appendChild(card);
        });
    }

    // Mock data generator for demonstration
    function getMockData(chordName, tuning) {
        // Just return some random shapes that look believable
        return {
            shapes: [
                {
                    name: "Position 1",
                    frets: [null, 3, 2, 0, 1, 0]
                },
                {
                    name: "Position 2",
                    frets: [8, 10, 10, 9, 8, 8]
                },
                {
                    name: "Position 3",
                    frets: [null, null, 10, 12, 13, 12]
                }
            ]
        };
    }
});
