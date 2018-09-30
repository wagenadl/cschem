#!/bin/sh

FN=$1

grep -q "This file is part of eln" $FN && exit 0

echo "Applying GPL text to $FN"

/bin/mv $FN $FN~
echo "// $FN - This file is part of eln" > $FN

cat >> $FN <<EOF

/* eln is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   eln is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with eln.  If not, see <http://www.gnu.org/licenses/>.
*/

EOF

cat $FN~ >> $FN
