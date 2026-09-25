import numpy as np
from scipy.optimize import minimize

def she_pwm_equations(alphas, M):
	"""
	Dynamically generates SHE-PWM equations for any pulse count N.
	Targets the Fundamental (n=1) and eliminates the lowest (N-1) non-triplen odd harmonics.
	"""
	N = len(alphas)
	out = []
		
	# 1. Generate the exact non-triplen odd harmonics list (e.g., 5, 7, 11, 13...)
	harmonics = []
	h = 3
	while len(harmonics) < N - 1:
		if h % 3 != 0:  # Skip triplen harmonics (3, 9, 15...) for 3-phase systems
			harmonics.append(h)
		h += 2
		
	# 2. Build the Fundamental (n=1) Equation
	v1 = 0
	for j in range(N):
		sign = -1 if (j % 2 != 0) else 1
		v1 += sign * np.cos(alphas[j])
	v1 -= M
	out.append(v1)
		
	# 3. Build Elimination Equations for each targeted harmonic
	for h_num in harmonics:
		v = 0
		for j in range(N):
			sign = -1 if (j % 2 != 0) else 1
			v += sign * np.cos(h_num * alphas[j])
		out.append(v)
		
	return out

def solve_she_pwm(N, M_target, max_attempts=100):
	"""
	Finds valid switching angles for an arbitrary N and Modulation Index.
	Uses multi-start optimization to ensure convergence.
	"""
	# Define physical boundaries (0 to 90 degrees in radians)
	bounds = [(0, np.pi/2) for _ in range(N)]
		
	# Objective function: Minimize sum of squared errors
	def objective(alphas):
		eqs = she_pwm_equations(alphas, M_target)
		return sum(e**2 for e in eqs)

	print(f"Searching for a valid physical solution for N={N}, M={M_target}...")
		
	for attempt in range(max_attempts):
		# Generate a sorted random guess to keep angles sequential initially
		guess = sorted(np.random.uniform(0.02, np.pi/2 - 0.02, N))
		
		res = minimize(objective, guess, bounds=bounds, method='L-BFGS-B')
		
		# Check if the optimization successfully forced equations to zero
		if res.fun < 1e-6:
			alphas_deg = np.degrees(res.x)
			
			# Verify strict physical constraints: 0 < a1 < a2 < ... < 90
			is_ordered = all(alphas_deg[i] < alphas_deg[i+1] for i in range(N-1))
			is_in_bounds = 0 < alphas_deg[0] and alphas_deg[-1] < 90
			
			if is_ordered and is_in_bounds:
				print(f"✨ Success! Found a solution on attempt {attempt + 1}:")
				return alphas_deg
				
	print(f"❌ Failed to find a valid physical solution within {max_attempts} attempts.")
	print("Note: A mathematical solution might not physically exist for this specific combination of N and M.")
	return None

# --- Configuration ---
N_pulses = 17
M_index = 0.60

angles = solve_she_pwm(N_pulses, M_index)

if angles is not None:
	print(f"\nModulation Index (M): {M_index}")

	out = "["

	for i, angle in enumerate(angles, 1):
		if (i > 1): out += ", "
		out += f"{angle:.4f}"
	
	out += "]"

	print(out)
