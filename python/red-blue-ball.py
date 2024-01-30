# You are given an urn containing N balls; n of them are red, and 100-n are green,
# where n is chosen uniformly at random in [0, N]. You take a random ball out of the urn
# —it’s red—and discard it. What is the chance of the next ball you pick (out of the 99 remaining) being red?
# According to Bayes' formula, the probabilities of "n" being each number in [0, N] become [0, 1, 2, ..., N] # before norm. 
# The next red ball has a probability of (m - 1) / (N - 1) when there are n red balls initially.

from sympy import symbols, summation, simplify

# Define the symbols
N, n, m, norm = symbols('N n m norm', interger=True)

norm = summation(n, (n, 1, N))
print(f"normalzation = {norm}")

red_ball_probability = summation(m * (m - 1), (m, 1, N)) / ((N - 1) * norm)

print(f"Red ball probability = {red_ball_probability}")

for i in range(2, 101, 10):
    ans = red_ball_probability.subs(N, i)
    print(f"N = {i}, red ball probability = {ans}")

red_ball_probability = simplify(red_ball_probability)
print(f"After simplify() red ball probability = {red_ball_probability}")