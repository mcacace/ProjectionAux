[Mesh]
  [cube]
    type = FileMeshGenerator
    file = 'mesh/minimal_example.e'
  []
  [rename_boundary]
    type = RenameBoundaryGenerator
    input = cube
    old_boundary = '1 2 3 4 5 6'
    new_boundary = 'back front right left bottom top'
  []
  [rename_blocks]
    type = RenameBlockGenerator
    input = rename_boundary
    old_block = '0 1 2'
    new_block = 'HD LD1 LD2'
  []
[]

[Variables]
  [u]
    [InitialCondition]
      type = ConstantIC
      value = 0.5
    []
  []
[]

[Kernels]
  [temp]
    type = CoefTimeDerivative
    variable = u
    Coefficient = 1
  []
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = mat
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    value = 1
    boundary = 'left'
  []
  [right]
    type = DirichletBC
    variable = u
    value = 0
    boundary = 'right'
  []
[]

[Materials]
  [mat]
    type = GenericConstantMaterial
    prop_names = 'mat'
    prop_values = 1
  []
[]

[AuxVariables]
  [pu_element]
    order = CONSTANT
    family = MONOMIAL
  []
  [pu_node]
  []
[]

[AuxKernels]
  [pu_element_aux]
    type = ProjectionModifiedAux
    variable = pu_element
    v = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [pu_node_aux]
    type = ProjectionModifiedAux
    variable = pu_node
    v = pu_element
    execute_on = 'INITIAL NONLINEAR_CONVERGENCE TIMESTEP_END'
  []
[]

[Preconditioning]
  [precond]
    type = SMP
    full = true
    petsc_options = '-snes_ksp_ew'
    petsc_options_iname = '-ksp_type -ksp_max_it
                           -pc_type
                           -snes_atol -snes_rtol -snes_max_it
                           -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = 'gmres 500
                           asm
                           1e-10 1e-10 200
                           lu NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  start_time = 0.0
  end_time = 1e5
  dt = 1e3
[]

[Outputs]
  print_linear_residuals = false
  exodus = true
[]
