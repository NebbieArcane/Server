#include <odb/pre.hxx>

#define ODB_COMMON_QUERY_COLUMNS_DEF
#include "account-odb.hxx"
#undef ODB_COMMON_QUERY_COLUMNS_DEF

namespace odb
{
  // toonExtra
  //

  template struct query_columns<
    ::Alarmud::toonExtra,
    id_common,
    access::object_traits_impl< ::Alarmud::toonExtra, id_common > >;

  const access::object_traits_impl< ::Alarmud::toonExtra, id_common >::
  function_table_type*
  access::object_traits_impl< ::Alarmud::toonExtra, id_common >::
  function_table[database_count];

  // toonRent
  //

  template struct query_columns<
    ::Alarmud::toonRent,
    id_common,
    access::object_traits_impl< ::Alarmud::toonRent, id_common > >;

  const access::object_traits_impl< ::Alarmud::toonRent, id_common >::
  function_table_type*
  access::object_traits_impl< ::Alarmud::toonRent, id_common >::
  function_table[database_count];

  // toon
  //

  template struct query_columns<
    ::Alarmud::toon,
    id_common,
    access::object_traits_impl< ::Alarmud::toon, id_common > >;

  const access::object_traits_impl< ::Alarmud::toon, id_common >::
  function_table_type*
  access::object_traits_impl< ::Alarmud::toon, id_common >::
  function_table[database_count];

  // user
  //

  template struct query_columns<
    ::Alarmud::user,
    id_common,
    access::object_traits_impl< ::Alarmud::user, id_common > >;

  const access::object_traits_impl< ::Alarmud::user, id_common >::
  function_table_type*
  access::object_traits_impl< ::Alarmud::user, id_common >::
  function_table[database_count];

  // userCount
  //

  const access::view_traits_impl< ::Alarmud::userCount, id_common >::
  function_table_type*
  access::view_traits_impl< ::Alarmud::userCount, id_common >::
  function_table[database_count];

  // legacy
  //

  template struct query_columns<
    ::Alarmud::legacy,
    id_common,
    access::object_traits_impl< ::Alarmud::legacy, id_common > >;

  const access::object_traits_impl< ::Alarmud::legacy, id_common >::
  function_table_type*
  access::object_traits_impl< ::Alarmud::legacy, id_common >::
  function_table[database_count];
}

#include <odb/post.hxx>
