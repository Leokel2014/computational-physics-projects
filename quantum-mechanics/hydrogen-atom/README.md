# Numerical Solution of the Schrödinger Equation — Hydrogen Atom

## Overview
This project implements a numerical solution of the radial Schrödinger equation for the hydrogen atom, focusing on bound-state solutions of the Coulomb potential.

The hydrogen atom is a cornerstone system in quantum mechanics and provides a direct connection between numerical methods and physical observables.

## Physics Background
By exploiting the spherical symmetry of the Coulomb potential, the three-dimensional Schrödinger equation reduces to a one-dimensional radial equation. This allows numerical investigation of atomic bound states and comparison with exact analytical energy levels.

## Method
- Reformulation of the Schrödinger equation in radial coordinates
- Discretization of the radial domain using finite-difference methods
- Construction of the effective radial Hamiltonian, including angular momentum terms
- Numerical solution of the resulting eigenvalue problem

## Results
The numerical energy eigenvalues reproduce the expected hydrogenic spectrum. Radial wavefunctions and probability densities exhibit the correct qualitative and quantitative behavior for different quantum numbers.

## Tools
Python, NumPy, Matplotlib

## Notes
This project was developed as part of undergraduate coursework in Quantum Mechanics.
