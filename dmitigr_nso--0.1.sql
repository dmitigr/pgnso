/* -*- SQL -*-
 * Copyright (C) 2020 Dmitry Igrishin
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Dmitry Igrishin
 * dmitigr@gmail.com
 */

set client_min_messages = warning;

drop type if exists textnso cascade;

create type textnso;

create function textnso_in(cstring)
  returns textnso
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_out(textnso)
  returns cstring
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_recv(internal)
  returns textnso
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_send(textnso)
  returns bytea
  language c immutable strict
  as '$libdir/dmitigr_nso';

create type textnso (
  input = textnso_in,
  output = textnso_out,
  receive = textnso_recv,
  send = textnso_send,
  collatable = true,
  internallength = variable,
  alignment = int4,
  category = 'S'
);

create cast (textnso as text) without function as assignment;
create cast (text as textnso) without function as assignment;

--------------------------------------------------------------------------------

create function textnso_cmp(textnso, textnso)
  returns integer
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_lt(textnso, textnso)
  returns boolean
  as '$libdir/dmitigr_nso'
  language c immutable strict;

create function textnso_le(textnso, textnso)
  returns boolean
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_eq(textnso, textnso)
  returns boolean
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_ne(textnso, textnso)
  returns boolean
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_ge(textnso, textnso)
  returns boolean
  language c immutable strict
  as '$libdir/dmitigr_nso';

create function textnso_gt(textnso, textnso)
  returns boolean
  language c immutable strict
  as '$libdir/dmitigr_nso';

create operator < (
  leftarg = textnso,
  rightarg = textnso,
  commutator = >,
  negator = >=,
  restrict = scalarltsel,
  join = scalarltjoinsel,
  procedure = textnso_lt
);

create operator <= (
  leftarg = textnso,
  rightarg = textnso,
  commutator = >=,
  negator = >,
  restrict = scalarlesel,
  join = scalarlejoinsel,
  procedure = textnso_le
);

create operator = (
  leftarg = textnso,
  rightarg = textnso,
  commutator = =,
  negator = <>,
  restrict = eqsel,
  join = eqjoinsel,
  hashes,
  merges,
  procedure = textnso_eq
);

create operator <> (
  leftarg = textnso,
  rightarg = textnso,
  commutator = <>,
  negator = =,
  restrict = neqsel,
  join = neqjoinsel,
  procedure = textnso_ne
);

create operator >= (
  leftarg = textnso,
  rightarg = textnso,
  commutator = <=,
  negator = <,
  restrict = scalargesel,
  join = scalargejoinsel,
  procedure = textnso_ge
);

create operator > (
  leftarg = textnso,
  rightarg = textnso,
  commutator = <,
  negator = <=,
  restrict = scalargtsel,
  join = scalargtjoinsel,
  procedure = textnso_gt
);

create operator family nso_fam using btree;

create operator class textnso_ops
  default for type textnso using btree family nso_fam
  as
    operator 1 <,
    operator 2 <=,
    operator 3 =,
    operator 4 >=,
    operator 5 >,
    function 1 textnso_cmp(textnso, textnso);
