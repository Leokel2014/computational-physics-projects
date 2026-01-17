# Numerical Solution of the Schrödinger Equation — Harmonic Oscillator

## Overview
This project presents a numerical solution of the time-independent Schrödinger equation for the one-dimensional quantum harmonic oscillator using finite-difference methods.

The harmonic oscillator is a fundamental model in quantum mechanics with wide applicability in atomic, molecular, and condensed matter physics.

## Physics Background
The quantum harmonic oscillator admits exact analytical solutions, making it an ideal system for validating numerical eigenvalue solvers and studying convergence properties of discretization methods.

## Method
- Discretization of the one-dimensional spatial domain
- Finite-difference approximation of the second derivative
- Construction of the Hamiltonian matrix including the quadratic potential term
- Numerical diagonalization of the Hamiltonian to obtain energy eigenvalues and eigenfunctions

## Results
The computed energy spectrum and eigenfunctions show excellent agreement with analytical solutions. Numerical results converge to exact values as the spatial grid resolution is increased.

## Tools
Python, NumPy, Matplotlib

## Notes
This project was developed as part of undergraduate coursework in Quantum Mechanics.
