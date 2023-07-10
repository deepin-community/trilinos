/*--------------------------------------------------------------------*/
/*    Copyright 2002 - 2008, 2010, 2011 National Technology &         */
/*    Engineering Solutions of Sandia, LLC (NTESS). Under the terms   */
/*    of Contract DE-NA0003525 with NTESS, there is a                 */
/*    non-exclusive license for use of this work by or on behalf      */
/*    of the U.S. Government.  Export of this program may require     */
/*    a license from the United States Government.                    */
/*--------------------------------------------------------------------*/

#ifndef MOCK_APPS_STK_SEND_ADAPTER_HPP
#define MOCK_APPS_STK_SEND_ADAPTER_HPP

#include <stk_search/BoundingBox.hpp>
#include <stk_search/IdentProc.hpp>
#include <stk_util/parallel/ParallelReduce.hpp>
#include "StkMesh.hpp"
#include <memory>
#include <utility>

namespace mock {

class StkSendAdapter
{
public:
  using EntityKey = StkMesh::EntityKey;
  using EntityProc = StkMesh::EntityProc;
  using EntityProcVec = StkMesh::EntityProcVec;
  using BoundingBox = StkMesh::BoundingBox;

  StkSendAdapter(MPI_Comm mpiComm, StkMesh& mockMesh, const std::string& fieldName)
   : m_comm(mpiComm),
     m_mesh(mockMesh),
     m_fieldName(fieldName)
  {}

  MPI_Comm comm() const {return m_comm;}

  void update_values() {called_update_values = true;}

  bool called_update_values = false;

  bool owning_rank() const { return stk::parallel_machine_rank(m_mesh.comm())==m_mesh.owning_rank(); }

  double get_field_value(const EntityKey & entityKey) const
  {
     return m_mesh.get_stk_field_value(entityKey, m_fieldName);
  }

  void bounding_boxes(std::vector<BoundingBox> & domain_vector) const
  {
    m_mesh.stk_source_bounding_boxes(domain_vector);
    int procInSearchComm = stk::parallel_machine_rank(m_comm);
    for(BoundingBox& box : domain_vector) {
      box.second.set_proc(procInSearchComm);
    }
  }

private:
  MPI_Comm m_comm;
  StkMesh& m_mesh;
  std::string m_fieldName;
};

}

#endif // MOCK_APPS_STK_SEND_ADAPTER_HPP
