#include "ProjectionModifiedAux.h"
#include "SystemBase.h"
#include "libmesh/system.h"

registerMooseObject("GolemApp", ProjectionModifiedAux);

InputParameters
ProjectionModifiedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Returns the specified variable as an auxiliary variable with a projection of the source "
      "variable. If they are the same type, this amounts to a simple copy. NOTE: so far the usage "
      "of lower dimensional elements is banned, since it gives a systematic segmentation fault "
      "error from libmesh System class.");
  params.addRequiredCoupledVar("v", "The variable to take the value from.");
  params.addParam<bool>("use_block_restriction_for_source",
                        false,
                        "Whether to use the auxkernel block restriction to also restrict the "
                        "locations selected for source variable values");
  params.set<bool>("_allow_nodal_to_elemental_coupling") = true;
  params.addParam<unsigned short>("ghost_layers", 1, "The number of layers of elements to ghost.");
  params.addRelationshipManager("ElementPointNeighborLayers",
                                Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<unsigned short>("layers") =
                                      obj_params.get<unsigned short>("ghost_layers");
                                  rm_params.set<bool>("use_displaced_mesh") =
                                      obj_params.get<bool>("use_displaced_mesh");
                                });
  return params;
}

ProjectionModifiedAux::ProjectionModifiedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _source_variable(*getFieldVar("v", 0)),
    _source_sys(_c_fe_problem.getSystem(coupledName("v"))),
    _use_block_restriction_for_source(getParam<bool>("use_block_restriction_for_source"))
{
  // Output some messages to user
  if (_source_variable.order() > _var.order())
    mooseInfo("Projection lowers order, please expect a loss of accuracy");
}

Real
ProjectionModifiedAux::computeValue()
{
  if (!isNodal() || (_source_variable.isNodal() && _source_variable.order() >= _var.order()))
  {
    return _v[_qp];
  }
  else if (isNodal() && _source_variable.getContinuity() != DISCONTINUOUS &&
           _source_variable.getContinuity() != SIDE_DISCONTINUOUS)
  {
    return _source_sys.point_value(
        _source_variable.number(), *_current_node, elemOnNodeVariableIsDefinedOn());
  }
  else
  {
    auto elem_ids = _mesh.nodeToElemMap().find(_current_node->id());
    mooseAssert(elem_ids != _mesh.nodeToElemMap().end(),
                "Should have found an element around node " + std::to_string(_current_node->id()));
    Real sum_weighted_values = 0;
    Real sum_volumes = 0;
    for (auto & id : elem_ids->second)
    {
      const auto & elem = _mesh.elemPtr(id);
      /// check if we are hitting a element defined on the local rank
      if (!isElementLocal(elem) || !elem->active())
        continue;
      const auto block_id = elem->subdomain_id();
      if (_source_variable.hasBlocks(block_id) &&
          (!_use_block_restriction_for_source || hasBlocks(block_id)))
      {
        const auto elem_volume = elem->volume();
        sum_weighted_values +=
            _source_sys.point_value(_source_variable.number(), *_current_node, elem) * elem_volume;
        sum_volumes += elem_volume;
      }
    }
    if (sum_volumes == 0)
      mooseError("Did not find a valid source variable value for node: ", *_current_node);
    return sum_weighted_values / sum_volumes;
  }
}

bool
ProjectionModifiedAux::isElementLocal(const Elem * element)
{
  /// we should only not be able to find an element if the mesh is distributed
  if (!element)
  {
    libmesh_assert(!_mesh.getMesh().is_serial());
    return false;
  }
  if (element->processor_id() == _communicator.rank())
    return true;
  return false;
}

const Elem *
ProjectionModifiedAux::elemOnNodeVariableIsDefinedOn() const
{
  for (const auto & elem_id : _mesh.nodeToElemMap().find(_current_node->id())->second)
  {
    const auto block_id = _mesh.elemPtr(elem_id)->subdomain_id();
    if (_source_variable.hasBlocks(block_id) && !_mesh.isLowerD(block_id) &&
        (!_use_block_restriction_for_source || hasBlocks(block_id)))
      return _mesh.elemPtr(elem_id);
  }
  mooseError("Source variable is not defined everywhere the target variable is");
}
