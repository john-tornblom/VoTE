#   Copyright (C) 2019 John Törnblom
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING. If not see
# <http://www.gnu.org/licenses/>.
.NOTPARALLEL:

EXTRA_DIST = setup.py vote.py build.py test.py

if HAVE_PYMOD_CFFI
all-local:
	$(PYTHON) setup.py build --build-lib $(builddir)

clean-local:
	$(PYTHON) setup.py clean --all
	rm -f $(builddir)/build/_vote.c
endif
