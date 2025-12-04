#pragma once

#include "AuxKernel.h"

namespace libMesh
{
class System;
}

class ProjectionModifiedAux : public AuxKernel
{
public:
  static InputParameters validParams();
  ProjectionModifiedAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _v;
  const MooseVariableFieldBase & _source_variable;
  const System & _source_sys;
  bool _use_block_restriction_for_source;

private:
  const Elem * elemOnNodeVariableIsDefinedOn() const;
  bool isElementLocal(const Elem * elem);
};
