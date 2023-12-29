# Copyright (©) 2023  Frosty515
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

.PHONY: clean-userland build-userland distclean-userland install-userland install-userland-headers

clean-userland:
	@$(MAKE) -C Libraries clean-libraries
	@$(MAKE) -C Programs clean-programs

build-userland:
	@$(MAKE) -C Libraries build-libraries
	@$(MAKE) -C Programs build-programs

distclean-userland:
	@$(MAKE) -C Libraries distclean-libraries
	@$(MAKE) -C Programs distclean-programs

install-userland:
	@$(MAKE) -C Libraries install-libraries
	@$(MAKE) -C Programs install-programs

install-userland-headers:
	@$(MAKE) -C Libraries install-library-headers