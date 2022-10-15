const EFFECT_IDS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100, 101, 102, 103, 104, 105, 106, 1000, 1001];
const EFFECT_NAMES = ['Static', 'Fade', 'Cycle', 'Dot', 'PingPong', 'Circle2D', 'Slide2D', 'Blink', 'Wipe', 'Shift', 'Firework', 'Firework2D', 'GIF 2D', 'Random Dots', 'Random Pattern', 'Sparkle Fill', 'Sparkle', 'Receive', 'Receive2D'];

var PALETTE_IDS = [];
var PALETTE_NAMES = [];

const host = window.location.hostname || '192.168.3.11';
var ws;

var currentCfg = {};

const events = new EventTarget();

const EVENT_VIRTUAL_DEVICES_CHANGED = 'event_virtual_devices_changed';
const EVENT_PALETTES_CHANGED = 'event_palettes_changed';
const EVENT_PRESETS_CHANGED = 'event_presets_changed';
const EVENT_ACTIVE_DEVICES_CHANGED = 'event_active_devices_changed';
const EVENT_PLAYLISTS_CHANGED = 'event_playlists_changed';
const EVENT_OVERLAY_CHANGED = 'event_overlay_changed';
const EVENT_SETTINGS_CHANGED = 'event_settings_changed';

const EVENT_LIVE_DATA = 'event_live_data';



class ComponentContainer {
    constructor(container, type) {
        this.container = container;
        this.type = type;
        this.components = {};

        var self = this;
        events.addEventListener(type.EVENT_NAME, function() {
            self.update(currentCfg[type.IDS_KEY]);
        });
    };

    addComponent(component, id) {
        this.components[id] = component;
    };

    removeComponent(id) {
        let component = this.components[id];
        delete this.components[id];
        return component;
    };

    getComponent(id) {
        return this.components[id];
    };

    getComponents() {
        return Object.values(this.components);
    };

    update(ids) {
        for (var id of ids) {
            if (!(id in this.components)) {
                var component = new this.type(id, this.container);
                this.components[id] = component;
                component.updateUI();
            }
        }

        for (var key in this.components) {
            if (!ids.includes(parseInt(key))) {
                let component = this.components[key];
                component.destroy();
                delete this.components[key];
            }
        }
    };
};

class Component {
    constructor(id, parent, eventName, idsKey, dataKey) {
        this.id = id;
        this.parent = parent;
        this.eventName = eventName;
        this.idsKey = idsKey;
        this.dataKey = dataKey;
        this.root = null;

        this.eventListener = this.updateUI.bind(this);
        events.addEventListener(this.eventName, this.eventListener);
    };

    destroy() {
        if (this.root && this.parent) {
            this.parent.removeChild(this.root);
        }
        events.removeEventListener(this.eventName, this.eventListener);
    };

    setRootHtml(html) {
        let componentId = this.getComponentId();
        let currentRoot = this.root || document.getElementById(componentId);
        if (currentRoot) {
            // replace
            currentRoot.outerHTML = html;
        } else {
            // insert
            this.parent.insertAdjacentHTML('beforeend', html);
        }
        this.root = document.getElementById(componentId);
    };

    setNameEditElememts(nameInput, editBtn) {
        this.nameInput = nameInput;
        this.editBtn = editBtn;

        var self = this;
        const submitName = function() {
            self.nameInput.contentEditable = 'false';
            let cfg = self.getConfig();

            const value = self.nameInput.innerHTML;
            cfg['n'] = value;
            ws.send(JSON.stringify({[self.dataKey]:[{id:self.id,n:value}]}));
            events.dispatchEvent(new Event(self.eventName));
        };
        this.nameInput.addEventListener("keydown", function(event) {
            if (event.keyCode === 13) {
                event.preventDefault();
                submitName();
            }
        });
        this.nameInput.addEventListener('focusout', (event) => {
            submitName();
        });
        this.nameInput.disabled = true;


        this.editBtn.onclick = function() {
            self.nameInput.contentEditable = 'true';
            self.nameInput.focus();
            var range = document.createRange();
            var sel = window.getSelection();

            let element = self.nameInput.childNodes[0] ?? self.nameInput;
            let offset = self.nameInput.childNodes[0]?.length ?? 0;
            range.setStart(element, offset);
            range.collapse(true);

            sel.removeAllRanges();
            sel.addRange(range);
        };
    };

    setDeleteElement(deleteBtn) {
        this.deleteBtn = deleteBtn;

        var self = this;
        this.deleteBtn.onclick = function() {
            const pIds = currentCfg[self.idsKey];
            const ps = currentCfg[self.dataKey];

            const idIndex = pIds.indexOf(self.id);
            if (idIndex > -1) {
                pIds.splice(idIndex, 1);
            }

            const pIndex = ps.map(p => p.id).indexOf(self.id);
            if (pIndex > -1) {
                ps.splice(pIndex, 1);
            }

            ws.send(JSON.stringify({[self.idsKey]:pIds}));
            events.dispatchEvent(new Event(self.eventName));
        };
    };

    getIds() {
        return currentCfg[this.idsKey] || [];
    };

    getConfig() {
        return (currentCfg[this.dataKey] || []).find(obj => obj['id'] == this.id) || null;
    };

    getFromConfig(key, defaultValue = null) {
        let cfg = this.getConfig();
        if (!(key in cfg) || !cfg[key]) {
            cfg[key] = defaultValue;
        }
        return cfg[key];
    };
};

class ExpandableComponent extends Component {

    constructor(id, parent, eventName, idsKey, dataKey) {
        super(id, parent, eventName, idsKey, dataKey);
        this.expanded = false;
    }

    setExpandableElements(element, checkbox) {
        this.expandableElement = element;
        this.expandableCheckbox = checkbox;

        var self = this;
        this.expandCheckbox.checked = this.expanded;
        this.expandCheckbox.onchange = function() {
            let value = self.expandCheckbox.checked;
            self.expand(value, true);
        };
    }

    expand(expanded, animate) {
        let element = this.expandableElement;
        let sectionHeight = element.scrollHeight;

        if (expanded) {
            if (animate) {
                element.style.height = sectionHeight + 'px';
            } else {
                var elementTransition = element.style.transition;
                element.style.transition = 'none';

                requestAnimationFrame(function() {
                    element.style.height = sectionHeight + 'px';

                    requestAnimationFrame(function() {
                        element.style.transition = elementTransition;
                    });
                });
            }
        } else {
            var elementTransition = element.style.transition;
            element.style.transition = 'none';

            if (animate) {
                requestAnimationFrame(function() {
                    element.style.height = sectionHeight + 'px';
                    element.style.transition = elementTransition;

                    requestAnimationFrame(function() {
                        element.style.height = 0 + 'px';
                    });
                });
            } else {
                requestAnimationFrame(function() {
                    element.style.height = 0 + 'px';

                    requestAnimationFrame(function() {
                        element.style.transition = elementTransition;
                    });
                });
            }
        }

        this.expanded = expanded;
    };
};

class PresetChipComponent extends Component {
    static EVENT_NAME = EVENT_PRESETS_CHANGED;
    static IDS_KEY = 'prstIds';
    static DATA_KEY = 'prsts';

    constructor(id, parent) {
        super(id, parent, PresetChipComponent.EVENT_NAME, PresetChipComponent.IDS_KEY, PresetChipComponent.DATA_KEY);
    };

    getComponentId() {
        return `preset_chip_${this.id}`;
    };

    setActive(active) {
        this.active = active;

        if (active) {
            this.chipButton.classList.add('chip-active');
        } else {
            this.chipButton.classList.remove('chip-active');
        }
    };

    updateUI() {
        let cfg = this.getConfig();
        if (!cfg) {
            return;
        }

        let html = `<button id="${this.getComponentId()}" class="chip">${cfg['n']}</button>`;
        this.setRootHtml(html);

        this.chipButton = this.root;

        var self = this;
        this.chipButton.onclick = function() {
            ws.send(JSON.stringify({prstId:self.id}));

            for (const component of presetChipsContainer.getComponents()) {
                component.setActive(false);
            }
            self.setActive(true);
        }
    };
};


class VirtualDeviceComponent extends Component {
    static EVENT_NAME = EVENT_VIRTUAL_DEVICES_CHANGED;
    static IDS_KEY = 'vdIds';
    static DATA_KEY = 'vds';

    constructor(id, parent) {
        super(id, parent, VirtualDeviceComponent.EVENT_NAME, VirtualDeviceComponent.IDS_KEY, VirtualDeviceComponent.DATA_KEY);
        this.paletteListener = this.updateUI.bind(this);
        events.addEventListener(EVENT_PALETTES_CHANGED, this.paletteListener);

        this.liveDataListener = this.onLiveData.bind(this);
        // live data preview on virtual device is disabled
        // events.addEventListener(EVENT_LIVE_DATA, this.liveDataListener);
    };

    destroy() {
        super.destroy();
        events.removeEventListener(EVENT_PALETTES_CHANGED, this.paletteListener);
        events.removeEventListener(EVENT_LIVE_DATA, this.liveDataListener);
    };

    getComponentId() {
        return `vd_${this.id}`;
    };

    onLiveData() {
        let element = this.root?.getElementsByClassName('vd')?.[0];
        let cfg = this.getConfig();

        if (!element || !cfg)
            return;

        let startIndex = cfg['sI'];
        let endIndex = cfg['eI'];

        let uint8Array = currentCfg['lD'];

        let bgStr = 'linear-gradient(to right';

        for (let i = startIndex * 3; i < endIndex * 3; i += 3) {
            const r = uint8Array[i];
            const g = uint8Array[i + 1];
            const b = uint8Array[i + 2];

            bgStr += `, rgb(${r},${g},${b})`
        }

        bgStr += ')';
        element.style.backgroundImage = bgStr;
    };

    updateUI() {
        let cfg = this.getConfig();
        if (!cfg) {
            return;
        }

        let numLeds = currentCfg["nLeds"] || 180;

        let html = `
        <div id="${this.getComponentId()}">
            <div class="card vd">
                <div class="card-header">
                    <h2 class="name-input">${cfg['n']}</h2>
                    <button class="edit-btn"></button>
                    <button class="delete-btn"></button>
                </div>
                <div class="vd-body">
                    <span class="grid-two-cols bri-slider-container" style="position: relative;">
                        <input type="range" min="0" max="255" value="${cfg['bri']}" class="bri-slider">
                    </span>
                    <label class="grid-two-cols">Index:</label>
                    <span class="multi-range grid-two-cols index-slider-container">
                        <input type="range" min="0" max="${numLeds}" value="${cfg['sI']}" class="def-slider dual-slider">
                        <input type="range" min="0" max="${numLeds}" value="${cfg['eI']}" class="def-slider dual-slider">
                        <output class="slider-output"></output>
                    </span>
                    <label class="grid-two-cols">Mirror:</label>
                    <input type="checkbox" class="grid-two-cols mirror-switch">
                    <select class="palette-dd"></select>
                    <select class="effect-dd"></select>
                    <div class="grid-two-cols effect-data-container"></div>
                </div>
            </div>
        </div>
        `;

        this.setRootHtml(html);

        this.nameInput = this.root.getElementsByClassName('name-input')[0];
        this.editBtn = this.root.getElementsByClassName('edit-btn')[0];
        this.deleteBtn = this.root.getElementsByClassName('delete-btn')[0];

        this.briSliderContainer = this.root.getElementsByClassName('bri-slider-container')[0];
        this.indexSliderContainer = this.root.getElementsByClassName('index-slider-container')[0];
        this.mirrorSwitch = this.root.getElementsByClassName('mirror-switch')[0];
        this.paletteDD = this.root.getElementsByClassName('palette-dd')[0];
        this.effectDD = this.root.getElementsByClassName('effect-dd')[0];
        this.effectDataContainer = this.root.getElementsByClassName('effect-data-container')[0];

        var self = this;
        this.setNameEditElememts(this.nameInput, this.editBtn);
        this.setDeleteElement(this.deleteBtn);

        this.createBriSlider(this.briSliderContainer, 0, 255, function() {
            const value = parseInt(this.value);
            cfg['bri'] = value;
            ws.send(JSON.stringify({vds:[{id:self.id,bri:value}]}));
        });

        this.createDualSlider(this.indexSliderContainer, 0, numLeds, cfg['sI'], cfg['eI'], function() {
            const value = parseInt(this.value);
            cfg['sI'] = value;
            ws.send(JSON.stringify({vds:[{id:self.id,sI:value}]}));
        }, function() {
            const value = parseInt(this.value);
            cfg['eI'] = value;
            ws.send(JSON.stringify({vds:[{id:self.id,eI:value}]}));
        });

        this.mirrorSwitch.onchange = function() {
            const value = this.checked;
            cfg['m'] = value;
            ws.send(JSON.stringify({vds:[{id:self.id,m:value}]}));
        };

        this.createDropdown(this.paletteDD, currentCfg['pIds'], currentCfg['ps']?.map(obj => obj['n']), cfg['pId'], function() {
            const value = parseInt(this.value);
            cfg['pId'] = value;
            ws.send(JSON.stringify({vds:[{id:self.id,pId:value}]}));
        });

        this.createDropdown(this.effectDD, EFFECT_IDS, EFFECT_NAMES, cfg['eId'], function() {
            const value = parseInt(this.value);
            cfg['eId'] = value;
            ws.send(JSON.stringify({vds:[{id:self.id,eId:value}]}));
            self.updateEffectDataUI();
        });

        this.updateEffectDataUI();
    };

    updateEffectDataUI() {
        let parent = this.effectDataContainer;
        parent.replaceChildren();

        let cfg = this.getConfig();
        if (!cfg) {
            return;
        }

        let eId = cfg['eId'];
        let eD = cfg['eD'] || {};

        let numLeds = currentCfg["nLeds"] || 180;

        let self = this;

        if (eId >= 1 && eId < 100) { // all cyclic effects
            const defD = (eD['d'] || 2000) / 100.0;
            const defSP = (eD['sP'] || 0) * 100;
            const defEP = (eD['eP'] || 1.0) * 100;

            const durationText = document.createElement('label');
            durationText.innerHTML = 'Duration:';
            parent.appendChild(durationText);

            const durationSlider = this.createSlider(10, 100, defD, function() {
                const value = parseInt(this.value) * 100;
                eD['d'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{d:value}}]}));
            });
            parent.appendChild(durationSlider);

            const posText = document.createElement('label');
            posText.innerHTML = 'Position:';
            parent.appendChild(posText);

            const posSlider = this.createEffectDualSlider(0, 100, defSP, defEP, function() {
                const value = parseInt(this.value) / 100.0;
                eD['sP'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{sP:value}}]}));
            }, function() {
                const value = parseInt(this.value) / 100.0;
                eD['eP'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{eP:value}}]}));
            });
            parent.appendChild(posSlider);
        } else if (eId >= 100) { // all simulation effects
            const defS = (eD['s'] || 1.0) * 10;

            const speedText = document.createElement('label');
            speedText.innerHTML = 'Speed:';
            parent.appendChild(speedText);

            const speedSlider = this.createSlider(1, 50, defS, function() {
                const value = parseInt(this.value) / 10.0;
                eD['s'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{s:value}}]}));
            });
            parent.appendChild(speedSlider);
        }

        // effect specific
        if (eId == 4 || eId == 5) {
            const defS = eD['s'] || 20.0;

            const sizeText = document.createElement('label');
            sizeText.innerHTML = 'Size:';
            parent.appendChild(sizeText);

            const sizeSlider = this.createSlider(1, numLeds, defS, function() {
                const value = parseInt(this.value);
                eD['s'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{s:value}}]}));
            });
            parent.appendChild(sizeSlider);
        } else if (eId == 7) {
            const sA = eD['sA'] || 0.0;
            const rPS = eD['rPS'] * 10.0 || 0.0;

            const angleText = document.createElement('label');
            angleText.innerHTML = 'Start Angle:';
            parent.appendChild(angleText);

            const angleSlider = this.createSlider(0, 359, sA, function() {
                const value = parseInt(this.value);
                eD['sA'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{sA:value}}]}));
            });
            parent.appendChild(angleSlider);

            const rotText = document.createElement('label');
            rotText.innerHTML = 'Rotations per second:';
            parent.appendChild(rotText);

            const rotSlider = this.createSlider(0, 20, rPS, function() {
                const value = parseInt(this.value) / 10.0;
                eD['rPS'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{rPS:value}}]}));
            });
            parent.appendChild(rotSlider);
        }  else if (eId == 8) {
            const nC = eD['nC'] || 2;

            const numText = document.createElement('label');
            numText.innerHTML = 'Number of Colors:';
            parent.appendChild(numText);

            const numSlider = this.createSlider(1, 20, nC, function() {
                const value = parseInt(this.value);
                eD['nC'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{nC:value}}]}));
            });
            parent.appendChild(numSlider);
        }  else if (eId == 9) {
            const nC = eD['nC'] || 2;

            const numText = document.createElement('label');
            numText.innerHTML = 'Number of Colors:';
            parent.appendChild(numText);

            const numSlider = this.createSlider(1, 20, nC, function() {
                const value = parseInt(this.value);
                eD['nC'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{nC:value}}]}));
            });
            parent.appendChild(numSlider);
        }  else if (eId == 10) {
            const nC = eD['nC'] || 2;

            const numText = document.createElement('label');
            numText.innerHTML = 'Number of Colors:';
            parent.appendChild(numText);

            const numSlider = this.createSlider(1, 20, nC, function() {
                const value = parseInt(this.value);
                eD['nC'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{nC:value}}]}));
            });
            parent.appendChild(numSlider);
        } else if (eId == 103) {
            const si = eD['si'] || 3;
            const d = eD['d'] * 100.0 || 0.5;

            const sizeText = document.createElement('label');
            sizeText.innerHTML = 'Size:';
            parent.appendChild(sizeText);

            const sizeSlider = this.createSlider(1, 100, si, function() {
                const value = parseInt(this.value);
                eD['si'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{si:value}}]}));
            });
            parent.appendChild(sizeSlider);

            const densityText = document.createElement('label');
            densityText.innerHTML = 'Density:';
            parent.appendChild(densityText);

            const densitySlider = this.createSlider(1, 100, d, function() {
                const value = parseInt(this.value) / 100.0;
                eD['d'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{d:value}}]}));
            });
            parent.appendChild(densitySlider);
        } else if (eId == 104) {
            const si = eD['si'] || 3;
            const d = eD['d'] * 100.0 || 0.5;
            const o = eD['o'] || 0;

            const sizeText = document.createElement('label');
            sizeText.innerHTML = 'Size:';
            parent.appendChild(sizeText);

            const sizeSlider = this.createSlider(1, 100, si, function() {
                const value = parseInt(this.value);
                eD['si'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{si:value}}]}));
            });
            parent.appendChild(sizeSlider);

            const densityText = document.createElement('label');
            densityText.innerHTML = 'Density:';
            parent.appendChild(densityText);

            const densitySlider = this.createSlider(1, 100, d, function() {
                const value = parseInt(this.value) / 100.0;
                eD['d'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{d:value}}]}));
            });
            parent.appendChild(densitySlider);

            const offsetText = document.createElement('label');
            offsetText.innerHTML = 'Offset:';
            parent.appendChild(offsetText);

            const offsetSlider = this.createSlider(1, 100, o, function() {
                const value = parseInt(this.value);
                eD['o'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{o:value}}]}));
            });
            parent.appendChild(offsetSlider);
        } else if (eId == 106) {
            const si = eD['si'] || 5;

            const sizeText = document.createElement('label');
            sizeText.innerHTML = 'Size:';
            parent.appendChild(sizeText);

            const sizeSlider = this.createSlider(1, 100, si, function() {
                const value = parseInt(this.value);
                eD['si'] = value;
                ws.send(JSON.stringify({vds:[{id:self.id,eD:{si:value}}]}));
            });
            parent.appendChild(sizeSlider);
        }
    };

    createSlider(min, max, current, oninput) {
        var element = document.createElement('span');
        element.style.position = 'relative';

        const output = document.createElement('output');
        output.classList.add('slider-output');

        const adjustBackground = function(val) {
            const perc = (val - min) / (max - min) * 100;

            const sliderWidth = slider.offsetWidth;
            const thumbWidth = slider.offsetHeight * 2; // TODO: fineadjust
            const pos = thumbWidth / 2.0 / sliderWidth * 100 + perc * (sliderWidth - thumbWidth) / sliderWidth;

            slider.style.background = `linear-gradient(to right, #c6bcbf ${pos}%, #0008 ${pos}%)`;


            const bubbleWidth = output.offsetWidth;
            const pos1 = -bubbleWidth / 2.0 + pos / 100.0 * sliderWidth;
            output.style.left = `${pos1}px`;
            output.innerHTML = `<span>${val}</span>`;
        };

        var slider = document.createElement('input');
        slider.type = 'range';
        slider.min = min;
        slider.max = max;
        slider.value = current;
        slider.onchange = oninput;
        slider.oninput = function() {
            var val = parseInt(slider.value);
            adjustBackground(val);
        };
        slider.classList.add('def-slider');
        element.appendChild(slider);

        element.appendChild(output);

        // needed to init background
        new ResizeObserver(function() {
            adjustBackground(slider.value);
        }).observe(slider);
        new ResizeObserver(function() {
            adjustBackground(slider.value);
        }).observe(output);


        return element;
    };

    createEffectDualSlider(min, max, current1, current2, oninput1, oninput2) {
        var element = document.createElement('span');
        element.classList.add('multi-range');

        var input1 = document.createElement('input');
        var input2 = document.createElement('input');

        const output = document.createElement('output');
        output.classList.add('slider-output');

        const adjustBackground = function() {
            const lower = input1.value;
            const upper = input2.value;

            const perc1 = (lower - min) / (max - min) * 100;
            const perc2 = (upper - min) / (max - min) * 100;

            const sliderWidth = input1.offsetWidth;
            const thumbWidth = input1.offsetHeight * 2;
            const pos1 = thumbWidth / 2.0 / sliderWidth * 100 + perc1 * (sliderWidth - thumbWidth) / sliderWidth;
            const pos2 = thumbWidth / 2.0 / sliderWidth * 100 + perc2 * (sliderWidth - thumbWidth) / sliderWidth;

            input1.style.background = `linear-gradient(to right, #0008 ${pos1}%, #c6bcbf ${pos1}%, #c6bcbf ${pos2}%, #0008 ${pos2}%) no-repeat center`;
            input1.style.backgroundOrigin = 'content-box';
        };

        const adjustBubble = function(val) {
            var pos = (val - min) / (max - min) * 100;
            var posOffset = Math.round(16 * pos / 100) + 4;

            output.style.left = `calc(${pos}% - ${posOffset}px)`;
            output.innerHTML = `<span>${val}</span>`;
        };

        input1.type = 'range';
        input1.min = min;
        input1.max = max;
        input1.value = current1;
        input1.oninput = function() {
            var lowerVal = parseInt(input1.value);
            var upperVal = parseInt(input2.value);

            if (lowerVal >= upperVal) {
                input1.value = lowerVal = upperVal - 1;
            }

            adjustBackground();
            adjustBubble(lowerVal);
        };
        input1.onchange = oninput1;
        input1.classList.add('def-slider');
        input1.classList.add('dual-slider');
        element.appendChild(input1);

        input2.type = 'range';
        input2.min = min;
        input2.max = max;
        input2.value = current2;
        input2.oninput = function() {
            var lowerVal = parseInt(input1.value);
            var upperVal = parseInt(input2.value);

            if (upperVal <= lowerVal) {
                input2.value = upperVal = lowerVal + 1;
            }

            adjustBackground();
            adjustBubble(upperVal);
        };
        input2.onchange = oninput2;
        input2.classList.add('def-slider');
        input2.classList.add('dual-slider');
        element.appendChild(input2);

        element.appendChild(output);

        new ResizeObserver(function() {
            adjustBackground();
        }).observe(input1);
        new ResizeObserver(function() {
            adjustBackground();
        }).observe(input2);

        return element;
    };

    createBriSlider(element, min, max, oninput) {
        let slider = element.querySelector('input');

        const adjustBackground = function(val) {
            const perc = (val - min) / (max - min) * 100;

            const sliderWidth = slider.offsetWidth;
            const thumbWidth = slider.offsetHeight;
            var pos = thumbWidth / 2.0 / sliderWidth * 100 + perc * (sliderWidth - thumbWidth) / sliderWidth;

            slider.style.background = `linear-gradient(to right, #c6bcbf ${pos}%, #0008 ${pos}%)`;
        };

        slider.onchange = oninput;
        slider.oninput = function() {
            var val = parseInt(slider.value);
            adjustBackground(val);
        };

        // needed to init background
        new ResizeObserver(function() {
            adjustBackground(slider.value);
        }).observe(slider);
    };

    createDualSlider(element, min, max, current1, current2, oninput1, oninput2) {
        let input1 = element.querySelector(':nth-child(1)');
        let input2 = element.querySelector(':nth-child(2)');

        const output = element.querySelector('output');

        const adjustBackground = function() {
            const lower = input1.value;
            const upper = input2.value;

            const perc1 = (lower - min) / (max - min) * 100;
            const perc2 = (upper - min) / (max - min) * 100;

            const sliderWidth = input1.offsetWidth;
            const thumbWidth = input1.offsetHeight * 2;
            const pos1 = thumbWidth / 2.0 / sliderWidth * 100 + perc1 * (sliderWidth - thumbWidth) / sliderWidth;
            const pos2 = thumbWidth / 2.0 / sliderWidth * 100 + perc2 * (sliderWidth - thumbWidth) / sliderWidth;

            input1.style.background = `linear-gradient(to right, #0008 ${pos1}%, #c6bcbf ${pos1}%, #c6bcbf ${pos2}%, #0008 ${pos2}%) no-repeat center`;
            input1.style.backgroundOrigin = 'content-box';
        };

        const adjustBubble = function(val) {
            var pos = (val - min) / (max - min) * 100;
            var posOffset = Math.round(16 * pos / 100) + 4;

            output.style.left = `calc(${pos}% - ${posOffset}px)`;
            output.innerHTML = `<span>${val}</span>`;
        };


        input1.oninput = function() {
            var lowerVal = parseInt(input1.value);
            var upperVal = parseInt(input2.value);

            if (lowerVal >= upperVal) {
                input1.value = lowerVal = upperVal - 1;
            }

            adjustBackground();
            adjustBubble(lowerVal);
        };
        input1.onchange = oninput1;

        input2.oninput = function() {
            var lowerVal = parseInt(input1.value);
            var upperVal = parseInt(input2.value);

            if (upperVal <= lowerVal) {
                input2.value = upperVal = lowerVal + 1;
            }

            adjustBackground();
            adjustBubble(upperVal);
        };
        input2.onchange = oninput2;

        new ResizeObserver(function() {
            adjustBackground();
        }).observe(input1);
        new ResizeObserver(function() {
            adjustBackground();
        }).observe(input2);
    };

    createDropdown(element, values, names, current, onchange) {
        for (var i = 0; i < values.length; i++) {
            var optionElem = document.createElement('option');
            optionElem.value = values[i];
            optionElem.text = names[i];
            element.appendChild(optionElem);
        }

        element.value = current;
        element.onchange = onchange;
    };
};


class PaletteComponent extends Component {
    static EVENT_NAME = EVENT_PALETTES_CHANGED;
    static IDS_KEY = 'pIds';
    static DATA_KEY = 'ps';

    constructor(id, parent) {
        super(id, parent, PaletteComponent.EVENT_NAME, PaletteComponent.IDS_KEY, PaletteComponent.DATA_KEY);
    };

    getComponentId() {
        return `palette_${this.id}`;
    };

    updateUI() {
        let cfg = this.getConfig();
        if (!cfg) {
            return;
        }

        let html = `
        <div id="${this.getComponentId()}" class="card">
            <div class="card-header">
                <h2 class="name-input">${cfg['n']}</h2>
                <button class="edit-btn"></button>
                <button class="delete-btn"></button>
            </div>
            <div class="card-body palette-body">
            </div>
        </div>
        `;

        this.setRootHtml(html);

        this.nameInput = this.root.getElementsByClassName('name-input')[0];
        this.editBtn = this.root.getElementsByClassName('edit-btn')[0];
        this.deleteBtn = this.root.getElementsByClassName('delete-btn')[0];
        this.grapick = new Grapick({el: this.root.getElementsByClassName('palette-body')[0], max: 99, height: 48});

        var self = this;
        this.setNameEditElememts(this.nameInput, this.editBtn);
        this.setDeleteElement(this.deleteBtn);


        for (const k of (cfg['ks'] || [])) {
            const p = k['p'];
            const c = k['c'];

            const pos = parseInt(p * 100);
            const color = '#' + c.toString(16).padStart(6, '0');

            this.grapick.addHandler(pos, color);
        }

        this.grapick.on('change', complete => {
            if (complete) {
                const ks = self.grapick.getHandlers().map(handler => {
                    const color = parseInt(handler.getColor().substring(1), 16);
                    const pos = handler.getPosition() / 100.0;
                    return {p:pos,c:color};
                });

                cfg['ks'] = ks;
                ws.send(JSON.stringify({ps:[{id:self.id,ks:ks}]}));
                events.dispatchEvent(new Event(EVENT_PALETTES_CHANGED));
            }
        });
    };
};


class PresetComponent extends ExpandableComponent {
    static EVENT_NAME = EVENT_PRESETS_CHANGED;
    static IDS_KEY = 'prstIds';
    static DATA_KEY = 'prsts';

    constructor(id, parent) {
        super(id, parent, PresetComponent.EVENT_NAME, PresetComponent.IDS_KEY, PresetComponent.DATA_KEY);
        this.activeDevicesListener = this.updateUI.bind(this);
        events.addEventListener(EVENT_ACTIVE_DEVICES_CHANGED, this.activeDevicesListener);
        this.expanded = false;
    };

    destroy() {
        super.destroy();
        events.removeEventListener(EVENT_ACTIVE_DEVICES_CHANGED, this.activeDevicesListener);
    };

    getComponentId() {
        return `preset_${this.id}`;
    };

    updateUI() {
        let cfg = this.getConfig();
        if (!cfg) {
            return;
        }

        let ea = cfg['eA'] || false;
        let activeIps = currentCfg['actDev'] || [];
        let remainingIPs = [];

        for (let ip of activeIps) {
            if (!(cfg['ips'] || []).includes(ip)) {
                remainingIPs.push(ip);
            }
        }

        let html = `
        <div id="${this.getComponentId()}">
            <div class="card">
                <div class="card-header">
                    <h2 class="name-input">${cfg['n']}</h2>
                    <button class="edit-btn"></button>
                    <button class="delete-btn"></button>
                </div>
                <div class="card-body">
                    <div id="preset-alexa-container">
                        <input type="checkbox" id="enable-alexa-checkbox"/>
                        <label for="enable-alexa-checkbox">Alexa</label>
                    </div>
                    <div>
                        <div class="active-ip-list">
                            ${(cfg['ips'] || []).map(ip => `
                                <div class="list-item">
                                    <span class="list-item-text">${ip}</span>
                                    <button class="list-item-delete-btn">x</button>
                                </div>`).join('')}
                        </div>
                        <div class="expandable-container remaining-ip-list">
                            <div class="divider"></div>
                            ${remainingIPs.map(ip => `
                                <div class="list-item">
                                    <span class="list-item-text">${ip}</span>
                                    <button class="list-item-add-btn">+</button>
                                </div>`).join('')}
                        </div>
                        <div class="card-btn-container">
                            <input type="checkbox" class="expand-checkbox">
                            <button class="use-btn">use</button>
                        </div>
                    </div>
                </div>
            </div>
        </div>`;

        this.setRootHtml(html);

        this.nameInput = this.root.getElementsByClassName('name-input')[0];
        this.editBtn = this.root.getElementsByClassName('edit-btn')[0];
        this.deleteBtn = this.root.getElementsByClassName('delete-btn')[0];
        this.alexaCheckbox = document.getElementById('enable-alexa-checkbox');
        this.activeIpList = this.root.getElementsByClassName('active-ip-list')[0];
        this.remainingIpList = this.root.getElementsByClassName('remaining-ip-list')[0];
        this.expandCheckbox = this.root.getElementsByClassName('expand-checkbox')[0];
        this.useBtn = this.root.getElementsByClassName('use-btn')[0];

        var self = this;
        this.setNameEditElememts(this.nameInput, this.editBtn);
        this.setDeleteElement(this.deleteBtn);
        this.setExpandableElements(this.remainingIpList, this.expandCheckbox);

        this.alexaCheckbox.checked = ea;
        this.alexaCheckbox.onchange = function() {
            const value = this.checked;
            ws.send(JSON.stringify({prsts:[{id:self.id,eA:value}]}));
        };

        for (let listItem of this.activeIpList.getElementsByClassName('list-item')) {
            let ip = listItem.getElementsByClassName('list-item-text')[0].innerHTML;
            let deleteBtn = listItem.getElementsByClassName('list-item-delete-btn')[0];

            deleteBtn.onclick = function() {
                let ips = self.getFromConfig('ips', []);
                const index = ips.indexOf(ip);
                if (index > -1) {
                    console.log(index);
                    ips.splice(index, 1);
                }

                ws.send(JSON.stringify({prsts:[{id:self.id,ips:ips}]}));
                self.updateUI();
            };
        }


        for (let listItem of this.remainingIpList.getElementsByClassName('list-item')) {
            let ip = listItem.getElementsByClassName('list-item-text')[0].innerHTML;
            let addBtn = listItem.getElementsByClassName('list-item-add-btn')[0];

            addBtn.onclick = function() {
                let ips = self.getFromConfig('ips', []);
                ips.push(ip);

                ws.send(JSON.stringify({prsts:[{id:self.id,ips:ips}]}));
                self.updateUI();
            };
        }

        this.useBtn.onclick = function() {
            ws.send(JSON.stringify({prstId:self.id}));
        }

        // init ui to current state
        this.expand(this.expanded, false);
    };
};


class PlaylistComponent extends ExpandableComponent {
    static EVENT_NAME = EVENT_PLAYLISTS_CHANGED;
    static IDS_KEY = 'pllstIds';
    static DATA_KEY = 'pllsts';

    constructor(id, parent) {
        super(id, parent, PlaylistComponent.EVENT_NAME, PlaylistComponent.IDS_KEY, PlaylistComponent.DATA_KEY);
        this.expanded = false;
    };

    getComponentId() {
        return `playlist_${this.id}`;
    };

    updateUI() {
        let cfg = this.getConfig();
        if (!cfg) {
            return;
        }

        let allPIds = currentCfg?.prstIds;
        let remainingPIds = [];

        for (let pId of allPIds) {
            if (!(cfg['pIds'] || []).includes(pId)) {
                remainingPIds.push(pId);
            }
        }

        let html = `
        <div id="${this.getComponentId()}">
            <div class="card">
                <div class="card-header">
                    <h2 class="name-input">${cfg['n']}</h2>
                    <button class="edit-btn"></button>
                    <button class="delete-btn"></button>
                </div>
                <div class="card-body">
                    <div>
                        <div class="active-preset-list">
                            ${(cfg['pIds'] || []).map(pId => `
                                <div class="list-item" pId="${pId}">
                                    <span class="list-item-text">${currentCfg?.prsts?.find(obj => obj.id == pId)?.n}</span>
                                    <button class="list-item-delete-btn">x</button>
                                </div>`).join('')}
                        </div>
                        <div class="expandable-container remaining-preset-list">
                            <div class="divider"></div>
                            ${remainingPIds.map(pId => `
                                <div class="list-item"" pId="${pId}">
                                    <span class="list-item-text">${currentCfg?.prsts?.find(obj => obj.id == pId)?.n}</span>
                                    <button class="list-item-add-btn">+</button>
                                </div>`).join('')}
                        </div>
                        <div class="card-btn-container">
                            <input type="checkbox" class="expand-checkbox">
                            <button class="use-btn">use</button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        `;

        this.setRootHtml(html);

        this.nameInput = this.root.getElementsByClassName('name-input')[0];
        this.editBtn = this.root.getElementsByClassName('edit-btn')[0];
        this.deleteBtn = this.root.getElementsByClassName('delete-btn')[0];
        this.activePresetList = this.root.getElementsByClassName('active-preset-list')[0];
        this.remainingPresetList = this.root.getElementsByClassName('remaining-preset-list')[0];
        this.expandCheckbox = this.root.getElementsByClassName('expand-checkbox')[0];
        this.useBtn = this.root.getElementsByClassName('use-btn')[0];

        var self = this;
        this.setNameEditElememts(this.nameInput, this.editBtn);
        this.setDeleteElement(this.deleteBtn);
        this.setExpandableElements(this.remainingPresetList, this.expandCheckbox);


        for (let listItem of this.activePresetList.getElementsByClassName('list-item')) {
            let pId = parseInt(listItem.getAttribute('pId'));
            let deleteBtn = listItem.getElementsByClassName('list-item-delete-btn')[0];

            deleteBtn.onclick = function() {
                let pIds = self.getFromConfig('pIds', []);
                const index = pIds.indexOf(pId);
                if (index > -1) {
                    console.log(index);
                    pIds.splice(index, 1);
                }

                ws.send(JSON.stringify({pllsts:[{id:self.id,pIds:pIds}]}));
                self.updateUI();
            };
        }


        for (let listItem of this.remainingPresetList.getElementsByClassName('list-item')) {
            let pId = parseInt(listItem.getAttribute('pId'));
            let addBtn = listItem.getElementsByClassName('list-item-add-btn')[0];

            addBtn.onclick = function() {
                let pIds = self.getFromConfig('pIds', []);
                pIds.push(pId);

                ws.send(JSON.stringify({pllsts:[{id:self.id,pIds:pIds}]}));
                self.updateUI();
            };
        }

        this.useBtn.onclick = function() {
            ws.send(JSON.stringify({prstId:self.id}));
        }

        // init ui to current state
        this.expand(this.expanded, false);
    };
};



let presetChipsContainer;
let vdsContainer;
let palettesContainer;
let presetsContainer;
let playlistsContainer;

let liveWs;

document.addEventListener("DOMContentLoaded", function(){
    console.log("DOM loaded");

    let presetChipsElement = document.getElementById('preset-chips');
    let vdsElement = document.getElementById('vds-list');
    let palettesElement = document.getElementById('palettes-list');
    let presetsElement = document.getElementById('presets-list');
    let playlistsElement = document.getElementById('playlists-list');

    presetChipsContainer = new ComponentContainer(presetChipsElement, PresetChipComponent);
    vdsContainer = new ComponentContainer(vdsElement, VirtualDeviceComponent);
    palettesContainer = new ComponentContainer(palettesElement, PaletteComponent);
    presetsContainer = new ComponentContainer(presetsElement, PresetComponent);
    playlistsContainer = new ComponentContainer(playlistsElement, PlaylistComponent);
    initOverlay();
    initSettings();

    navigation(0);

    init();
});

function initOverlay() {
    events.addEventListener(EVENT_OVERLAY_CHANGED, initOverlay);

    let transitionSelect = document.getElementById('overlay-transition-select');
    let transitionDuration = document.getElementById('overlay-transition-duration');
    let dataSelect = document.getElementById('overlay-data-select');

    let countdownMinutes = document.getElementById('overlay-countdown-minutes');
    let countdownSeconds = document.getElementById('overlay-countdown-seconds');
    let startBtn = document.getElementById('overlay-start-btn');

    transitionSelect.onchange = function() {
        let id = parseInt(this.value);
        ws.send(JSON.stringify({tId:id}));
    }

    dataSelect.onchange = function() {
        let id = parseInt(this.value);
        ws.send(JSON.stringify({oId:id}));
    }

    let transitionDurationChanged = function() {
        let duration = parseInt(this.value);
        ws.send(JSON.stringify({tD:{dMs:duration}}));
    }

    transitionDuration.addEventListener('focusout', transitionDurationChanged);
    transitionDuration.onchange = transitionDurationChanged;


    startBtn.onclick = function() {
        let mins = parseInt(countdownMinutes.value);
        let secs = parseInt(countdownSeconds.value);
        let valueMs = (mins * 60 + secs) * 1000;

        ws.send(JSON.stringify({oD:{vMs:valueMs}}));
    }


    transitionSelect.value = currentCfg['tId'] || 1;
    dataSelect.value = currentCfg['oId'] || 1;
}

function initSettings() {
    let numLedsInput = document.getElementById('settings-num-leds');

    let numLedsChanged = function() {
        let numLeds = parseInt(this.value);
        ws.send(JSON.stringify({nLeds:numLeds}));
    }

    numLedsInput.addEventListener('focusout', numLedsChanged);
    numLedsInput.onChange = numLedsChanged;


    let alexaNameInput = document.getElementById('settings-alexa-name');

    let alexaNameChanged = function() {
        let alexaName = this.value;

        console.log('Alexa name changed: ' + alexaName);

        ws.send(JSON.stringify({an:alexaName}));
    }

    alexaNameInput.addEventListener('focusout', alexaNameChanged);
    alexaNameInput.onchange = alexaNameChanged;


    let lineChart;

    let onChartChange = function() {
        let data = [];
        for (index in lineChart.data.datasets[0].data) {
            entry = lineChart.data.datasets[0].data[index];

            data.push({
                t: entry.x,
                b: entry.y / 100.0
            });
        }

        ws.send(JSON.stringify({briCtrls: data}));
    }

    let data = [
        {
            x: 0,
            y: 50
        },
        {
            x: 24,
            y: 50
        }
    ];

    let chart = document.getElementById('bri-control-chart');
    let gradient = chart.getContext('2d').createLinearGradient(0, 0, 0, 450);

    gradient.addColorStop(0, 'rgba(255, 0,0, 0.5)');
    gradient.addColorStop(0.5, 'rgba(255, 0, 0, 0.25)');
    gradient.addColorStop(1, 'rgba(255, 0, 0, 0)');

    const config = {
        responsive: true,
        plugins:{
            legend: {
                display: false
            },
            tooltip: {
                displayColors: false,
                callbacks: {
                    title: function(context) {
                        let val = context[0].parsed.x;
                        let hour = Math.floor(val);
                        let frac = val - hour;
                        let min = Math.floor(frac * 60);

                        let str = `${hour}:${(min).toLocaleString('en-US', {minimumIntegerDigits: 2, useGrouping:false})}`;
        
                        return str;
                    },
                    label: function(context) {
                        let label = `${Math.round(context.parsed.y)}%`;
                        return label;
                    }
                }
            },
            dragData: {        
                round: 1,
                dragX: true,
                showTooltip: false,
                onDragStart:function (e, datasetIndex, index, value) {
                },
                onDrag: function (e, datasetIndex, index, value) {
                    e.target.style.cursor = 'grabbing'
                    //console.log("Drag Value: ", value.x)

                    let prevVal = lineChart.data.datasets[datasetIndex].data[index - 1]?.x ?? -1.0;
                    let nextVal = lineChart.data.datasets[datasetIndex].data[index + 1]?.x ?? 25.0;

                    return value.x > prevVal && value.x < nextVal;
                },
                onDragEnd: function (e, datasetIndex, index, value) {
                    e.target.style.cursor = 'default'
                    onChartChange();
                },
            },
        },
        scales: {
            x:{
                type: 'linear',
                min: 0,
                max: 24,
                ticks: {
                    stepSize: 1,
                    color: '#ddd',
                    callback(value, index) {
                        let str = `${value}:00`;
                        return index % 2 == 0 ? str : '';
                    }
                },
                grid: {
                    color: '#666'
                }
            },
            y:{
                beginAtZero: true,
                steps: 1,
                stepValue: 1,
                max: 100,
                ticks: {
                    color: '#ddd',
                    callback(value) {
                        return value + '%';
                    }
                },
                grid: {
                    color: '#666'
                }
            },
        },
        onClick: function (element, dataAtClick) {
            let dataIndex = dataAtClick[0]?.index;
            if (dataIndex !== undefined) {
                // handle delete
                this.data.datasets[0].data.splice(dataIndex, 1);
            } else {
                let scaleRef, valueX, valueY;
                for (var scaleKey in this.scales) {
                    scaleRef = this.scales[scaleKey];
                    if (scaleRef.isHorizontal()) {
                        valueX = scaleRef.getValueForPixel(element.native.offsetX);
                    } else {
                        valueY = scaleRef.getValueForPixel(element.native.offsetY);
                    }
                }

                this.data.datasets[0].data.sort(function(a, b) {
                    return a.x - b.x;
                });

                var index = 0;
                while (index < this.data.datasets[0].data.length && this.data.datasets[0].data[index].x < valueX) {
                    index += 1;
                }

                this.data.datasets[0].data.splice(index, 0, {
                    x: valueX,
                    y: valueY
                });
            }
            this.update('none');
            onChartChange();
        },
    };

    
    lineChart = new Chart(chart, {
        type: 'line',
        data: {
            datasets: [
                {
                    data: data,
                    backgroundColor: '#AD35BA',
                    borderColor: '#AD35BA',
                    borderWidth: 4,
                    pointHitRadius: 10,
                    pointRadius: 4,
                    pointHoverRadius: 8,
                    pointBorderColor: "#fff",
                    pointBackgroundColor: "rgba(173, 53, 186, 0.1)",
                }
            ]
        },
        options: config
    });


    let useBriControlCheckbox = document.getElementById('settings-use-bri-control');
    useBriControlCheckbox.onchange = function(e) {
        let value = e.target.checked;
        chart.style.display = value ? 'block' : 'none';

        currentCfg['briCtrl'] = value;
        ws.send(JSON.stringify({briCtrl: value}));
    }

    events.addEventListener(this.EVENT_SETTINGS_CHANGED, function() {
        alexaNameInput.value = currentCfg['an'] || '';

        useBriControlCheckbox.checked = currentCfg['briCtrl'];
        chart.style.display = currentCfg['briCtrl'] ? 'block' : 'none';

        let chartData = [];
        for (index in currentCfg['briCtrls']) {
            let entry = currentCfg['briCtrls'][index];

            chartData.push({
                x: entry.t,
                y: entry.b * 100.0
            });
        }

        lineChart.data.datasets[0].data = chartData;
        lineChart.update();
    });
}

function init() {
    ws = new WebSocket(`ws://${host}/ws`);

    ws.onopen = function() {
        console.log("ws opened");
    };

    ws.onmessage = function(evt) {
        console.log("ws message");

        var msg = evt.data;
        console.log(msg);

        const cfg = JSON.parse(msg);

        const vdIds = cfg['vdIds'];
        const vds = cfg['vds'];
        const pIds = cfg['pIds'];
        const ps = cfg['ps'];
        const prstIds = cfg['prstIds'];
        const prsts = cfg['prsts'];
        const actDev = cfg['actDev'];
        const pllstIds = cfg['pllstIds'];
        const pllsts = cfg['pllsts'];
        const on = cfg['on'] || false;

        const tId = cfg['tId'] || 0;
        const oId = cfg['oId'] || 0;

        const nLeds = cfg['nLeds'] || 0;

        const an = cfg['an'] || '';

        const briCtrl = cfg['briCtrl'] || false;
        const briCtrls = cfg['briCtrls'];

        if (pIds || ps) {
            currentCfg['pIds'] = pIds || currentCfg['pIds'];
            currentCfg['ps'] = ps || currentCfg['ps'];
            events.dispatchEvent(new Event(EVENT_PALETTES_CHANGED));
        }

        if (prstIds || prsts) {
            currentCfg['prstIds'] = prstIds || currentCfg['prstIds'];
            currentCfg['prsts'] = prsts || currentCfg['prsts'];
            events.dispatchEvent(new Event(EVENT_PRESETS_CHANGED));
        }

        if (actDev) {
            currentCfg['actDev'] = actDev;
            events.dispatchEvent(new Event(EVENT_ACTIVE_DEVICES_CHANGED));
        }

        if (pllstIds || pllsts) {
            currentCfg['pllstIds'] = pllstIds || currentCfg['pllstIds'];
            currentCfg['pllsts'] = pllsts || currentCfg['pllsts'];
            events.dispatchEvent(new Event(EVENT_PLAYLISTS_CHANGED));
        }

        if (vdIds || vds) {
            currentCfg['vdIds'] = vdIds || currentCfg['vdIds'];
            currentCfg['vds'] = vds || currentCfg['vds'];
            events.dispatchEvent(new Event(EVENT_VIRTUAL_DEVICES_CHANGED));
        }

        if ('on' in cfg) {
            currentCfg['on'] = on;
        }

        if (tId) {
            currentCfg['tId'] = tId;
        }

        if (oId) {
            currentCfg['oId'] = oId;
        }

        if (tId || oId) {
            events.dispatchEvent(new Event(EVENT_OVERLAY_CHANGED));
        }

        if (nLeds) {
            currentCfg['nLeds'] = nLeds;
            events.dispatchEvent(new Event(EVENT_VIRTUAL_DEVICES_CHANGED)); // need to update indices
        }

        if (an) {
            currentCfg['an'] = an;
            events.dispatchEvent(new Event(this.EVENT_SETTINGS_CHANGED));
        }

        if ('briCtrl' in cfg) {
            currentCfg['briCtrl'] = briCtrl;
        }

        if (briCtrls) {
            currentCfg['briCtrls'] = briCtrls;
        }

        if ('briCtrl' in cfg || briCtrls) {
            // this.EVENT_... is needed here for some reason
            events.dispatchEvent(new Event(this.EVENT_SETTINGS_CHANGED));
        }

        console.log("init finished");

        let powerBtn = document.getElementById('power-btn');
        if (on) {
            powerBtn.classList.remove('vd-btn-off');
            powerBtn.classList.add('vd-btn-on');
        } else {
            powerBtn.classList.remove('vd-btn-on');
            powerBtn.classList.add('vd-btn-off');
        }
    };

    ws.onclose = function() {
        console.log("ws closed");

        // auto reconnect
        setTimeout(function() {
            init();
        }, 1000);
    };
};

function toggleLiveData(e) {
    const value = !(liveWs);

    if (value) {
        enableLiveData();
    } else {
        disableLiveData();
    }

    console.log(value);

    if (value) {
        e.classList.remove('vd-btn-off');
        e.classList.add('vd-btn-on');
    } else {
        e.classList.remove('vd-btn-on');
        e.classList.add('vd-btn-off');
    }
}

function enableLiveData() {
    liveWs = new WebSocket(`ws://${host}/ws/live`);

    liveWs.onopen = function() {
        console.log("liveWs opened");
    };

    liveWs.onmessage = function(evt) {
        var msg = evt.data;
        msg.arrayBuffer().then(buffer => {
            var uint8Array = new Uint8Array(buffer);
            currentCfg['lD'] = uint8Array;
            events.dispatchEvent(new Event(EVENT_LIVE_DATA));

            let gradStr = 'linear-gradient(to right';

            for (let i = 0; i < uint8Array.length; i += 3) {
                const r = uint8Array[i];
                const g = uint8Array[i + 1];
                const b = uint8Array[i + 2];

                gradStr += `, rgb(${r},${g},${b})`
            }

            gradStr += ')';

            const testGradient = document.getElementsByClassName('header')[0];
            testGradient.style.backgroundImage = gradStr;
        });
    };

    liveWs.onclose = function() {
        console.log("liveWs closed");
    };
};

function disableLiveData() {
    if (liveWs) {
        liveWs.close();
        liveWs = null;
    }

    // reset the background
    document.getElementsByClassName('header')[0].style.backgroundImage = 'linear-gradient(to right, #16999E, #4f0b48)';
}

function generateId(disjunct) {
    if (!disjunct) {
        disjunct = [];
    }

    var id;
    do {
        // id = parseInt(Math.random() * 0xFFFFFFFF + 1); // no 0 // 32bit breaks ArduinoJson
        id = parseInt(Math.random() * 0x7FFFFFFF + 1); // use 31bit
    } while(disjunct.includes(id));

    return id;
};

function addComponent(idsKey, dataKey, prefix, cfg, event) {
    const ids = currentCfg[idsKey];
    const data = currentCfg[dataKey];

    const id = generateId(ids);
    ids.push(id);

    cfg['id'] = id;
    cfg['n'] = `${prefix} #${id}`;
    data.push(cfg);

    ws.send(JSON.stringify({[idsKey]:ids,[dataKey]:[cfg]}));
    events.dispatchEvent(new Event(event));
};

function addVD() {
    addComponent('vdIds', 'vds', 'VD', {sI:0,eI:20,eId:EFFECT_IDS[0],pId:PALETTE_IDS[0]}, EVENT_VIRTUAL_DEVICES_CHANGED);
};

function addPalette() {
    addComponent('pIds', 'ps', 'Palette', {}, EVENT_PALETTES_CHANGED);
};

function addPreset() {
    addComponent('prstIds', 'prsts', 'Preset', {}, EVENT_PRESETS_CHANGED);
};

function addPlaylist() {
    addComponent('pllstIds', 'pllsts', 'Playlist', {}, EVENT_PLAYLISTS_CHANGED);
};

function toggleOnOff(e) {
    console.log('on/off');

    const value = !e.classList.contains('vd-btn-on');
    ws.send(JSON.stringify({on:value}));

    if (value) {
        e.classList.remove('vd-btn-off');
        e.classList.add('vd-btn-on');
    } else {
        e.classList.remove('vd-btn-on');
        e.classList.add('vd-btn-off');
    }
};

function navigation(val) {
    console.log(`navigate to ${val}`);

    const mainElements = [
        document.getElementById('vds-div'),
        document.getElementById('palettes-div'),
        document.getElementById('presets-div'),
        document.getElementById('playlists-div'),
        document.getElementById('overlay-div'),
        document.getElementById('settings-div')
    ];

    const mainElementsVisible = [
        [true, false, false, false, false, false],
        [false, true, false, false, false, false],
        [false, false, true, true, false, false],
        [false, false, false, false, true, false],
        [false, false, false, false, false, true]
    ];

    const currentMainElementsVisible = mainElementsVisible[val];
    for (let i = 0; i < mainElements.length; i++) {
        mainElements[i].style.display = currentMainElementsVisible[i] ? 'block' : 'none';
    }
};
