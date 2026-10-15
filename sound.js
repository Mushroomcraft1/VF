export { setSoundOptions, startSound, stopSound };

let soundStarted = false;

let soundOptions = {
	singlePhase: false,
	sampleRate: 48000,
	bufferDuration: 50,
	waveAmplitude: 0.1,
	smoothLength: 5,
};

/**
 * @param {Object}
 */
function setSoundOptions(newOptions) {
	for (const key in newOptions) {
		soundOptions[key] = newOptions[key]
	}
}


async function startSound() {
	if (soundStarted) throw "SoundSim already started";
	soundStarted = true;
	console.log("Started sound")

	const audioCtx = new AudioContext();
	audioCtx.sampleRate = soundOptions.sampleRate;

	await audioCtx.audioWorklet.addModule("processor.js");
	const noiseNode = new AudioWorkletNode(audioCtx, "vf-sim");
	noiseNode.connect(audioCtx.destination);
}

function stopSound() {
	clearInterval(soundLoop);
	soundStarted = false;
}