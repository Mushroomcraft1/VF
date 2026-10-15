

/**
 * Counts the number of 1 bits in an integer
 * @param {Integer} x 
 * @returns 
 */
function popCount(x) {
	let n = 0;
	while (x) {
		x &= x - 1;
		++n;
	}
	return n ;
}

class StreamProcessor extends AudioWorkletProcessor {
	process(inputs, outputs, parameters) {
		const output = outputs[0];

		const nowBuffering = output[0];
		const len = nowBuffering.length;

		for (let i = 0; i < nowBuffering.length; ++i) {

			nowBuffering[i] = Math.sin((i + idx) / 1);
			
			// prevPulse = currentPulse;
		}
		idx += len;
		

		return true;
	}
}

registerProcessor("vf-sim", StreamProcessor);