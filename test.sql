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

do $$
declare
  str textnso;
begin
  perform 1 where ''::textnso = ''::textnso;
  assert(FOUND);
  perform 1 where not ''::textnso <> ''::textnso;
  assert(FOUND);

  for str in select 'text'::textnso
             union all
             select 'текст'::textnso
  loop
    perform 1 where '' < str;
    assert(FOUND);
    perform 1 where '' <= str;
    assert(FOUND);

    perform 1 where str > '';
    assert(FOUND);
    perform 1 where str >= '';
    assert(FOUND);

    perform 1 where str = str;
    assert(FOUND);
    perform 1 where (str||'str1')::textnso = (str||'str1')::textnso;
    assert(FOUND);
    perform 1 where (str||'str01')::textnso = (str||'str1')::textnso;
    assert(FOUND);
    perform 1 where (str||'str10')::textnso = (str||'str010')::textnso;
    assert(FOUND);
    perform 1 where (str||'str010')::textnso = (str||'str010')::textnso;
    assert(FOUND);
    perform 1 where (str||'str0010')::textnso = (str||'str010')::textnso;
    assert(FOUND);
    perform 1 where ''::textnso <> str;
    assert(FOUND);
    perform 1 where (str||'1'||str)::textnso <> (str||'0'||str)::textnso;
    assert(FOUND);

    perform 1 where str < (str||'1')::textnso;
    assert(FOUND);
    perform 1 where str <= (str||'1')::textnso;
    assert(FOUND);
    perform 1 where (str||'str1')::textnso < (str||'str2')::textnso;
    assert(FOUND);
    perform 1 where (str||'str1')::textnso <= (str||'str2')::textnso;
    assert(FOUND);
    perform 1 where (str||'str9')::textnso < (str||'str10')::textnso;
    assert(FOUND);
    perform 1 where (str||'str9')::textnso <= (str||'str10')::textnso;
    assert(FOUND);
    perform 1 where (str||'str0')::textnso < (str||'str1')::textnso;
    assert(FOUND);
    perform 1 where (str||'str0')::textnso <= (str||'str1')::textnso;
    assert(FOUND);
    perform 1 where (str||'str01')::textnso < (str||'str2')::textnso;
    assert(FOUND);
    perform 1 where (str||'str01')::textnso <= (str||'str2')::textnso;
    assert(FOUND);
    perform 1 where (str||'str00')::textnso < (str||'str02')::textnso;
    assert(FOUND);
    perform 1 where (str||'str00')::textnso <= (str||'str02')::textnso;
    assert(FOUND);
    perform 1 where (str||'str09')::textnso < (str||'str10')::textnso;
    assert(FOUND);
    perform 1 where (str||'str09')::textnso <= (str||'str10')::textnso;
    assert(FOUND);
    perform 1 where (str||'str010')::textnso < (str||'str090')::textnso;
    assert(FOUND);
    perform 1 where (str||'str010')::textnso <= (str||'str090')::textnso;
    assert(FOUND);

    perform 1 where (str||'str1')::textnso > (str||'str')::textnso;
    assert(FOUND);
    perform 1 where (str||'str1')::textnso >= (str||'str')::textnso;
    assert(FOUND);
    perform 1 where (str||'str2')::textnso > (str||'str1')::textnso;
    assert(FOUND);
    perform 1 where (str||'str2')::textnso >= (str||'str1')::textnso;
    assert(FOUND);
    perform 1 where (str||'str10')::textnso > (str||'str9')::textnso;
    assert(FOUND);
    perform 1 where (str||'str10')::textnso >= (str||'str9')::textnso;
    assert(FOUND);
    perform 1 where (str||'str1')::textnso > (str||'str0')::textnso;
    assert(FOUND);
    perform 1 where (str||'str1')::textnso >= (str||'str0')::textnso;
    assert(FOUND);
    perform 1 where (str||'str2')::textnso > (str||'str01')::textnso ;
    assert(FOUND);
    perform 1 where (str||'str2')::textnso >= (str||'str01')::textnso ;
    assert(FOUND);
    perform 1 where (str||'str02')::textnso > (str||'str00')::textnso;
    assert(FOUND);
    perform 1 where (str||'str02')::textnso >= (str||'str00')::textnso;
    assert(FOUND);
    perform 1 where (str||'str10')::textnso > (str||'str09')::textnso;
    assert(FOUND);
    perform 1 where (str||'str10')::textnso >= (str||'str09')::textnso;
    assert(FOUND);
    perform 1 where (str||'str090')::textnso > (str||'str010')::textnso;
    assert(FOUND);
    perform 1 where (str||'str090')::textnso >= (str||'str010')::textnso;
    assert(FOUND);
  end loop;
end$$;
