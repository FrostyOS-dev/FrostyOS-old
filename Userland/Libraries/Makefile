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

.PHONY: clean-libraries build-libraries distclean-libraries install-libraries install-library-headers

clean-libraries:
	@$(MAKE) -C LibC clean-libc

build-libraries:
	@echo -------------
	@echo Building LibC
	@echo -------------
	@$(MAKE) -C LibC build-libc

distclean-libraries:
	@$(MAKE) -C LibC distclean-libc

install-libraries:
	@$(MAKE) -C LibC install-libc

install-library-headers:
	@$(MAKE) -C LibC install-libc-headers
	