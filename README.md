objspace_helpers
================

Experimental repository containing helper functions for ObjectSpace in MRI Ruby 2.1+.

This repository uses a hand-crafted C extension which is not advised for production
or even day-by-day development use. It uses private APIs and stuff.


Installation
------------

```
gem install objspace_helpers
```


Usage
-----

```ruby
obj = nil
obj_addrs = ObjspaceHelpers.diff do
  obj = "mystring"
  # something else
end
obj_addrs.include?(ObjspaceHelpers.address_of(obj)) # => true
```

Authors
-------

- [Lukas Fittl](mailto:lukas@fittl.com)

License
-------

Copyright (c) 2014, Lukas Fittl <lukas@fittl.com><br>
objspace_helpers is licensed under the 2-clause BSD license, see LICENSE file for details.
