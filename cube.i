[Mesh]
  [cube]
    type = FileMeshGenerator
    file = 'mesh/cube.e'
  []
  [rename_boubdary]
    type = RenameBoundaryGenerator
    input = cube
    old_boundary = '1 2 3 4 5 6'
    new_boundary = 'back front right left bottom top'
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
  [mat_element]
    order = CONSTANT
    family = MONOMIAL
  []
  [mat_node]
  []
[]

[AuxKernels]
  [mat_element_aux]
    type = MaterialRealAux
    variable = mat_element
    property = mat
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [mat_node_aux]
    type = ProjectionAux
    variable = mat_node
    v = mat_element
    execute_on = 'INITIAL TIMESTEP_END'
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
  end_time = 1
  dt = 1
[]

[Outputs]
  exodus = true
[]
