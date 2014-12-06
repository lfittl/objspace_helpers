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
class SampleClass
  def initialize
    @@my_class_variable = "yay"
  end
end

top_level_leaks, leak_sources = ObjspaceHelpers.find_leak_sources(trace: true) do
  SampleClass.new

  'my_symbol'.to_sym
end

top_level_leaks
 => [#<ObjspaceHelpers::TrackedObject:0x007f96c9770490 @address=140285595807160, @referenced_by=[]>]

top_level_leaks.first.info
 => {"type"=>"STRING", "class"=>140285594211920, "frozen"=>true, "embedded"=>true, "fstring"=>true, "bytesize"=>9, "value"=>"my_symbol", "encoding"=>"US-ASCII", "references"=>[], "file"=>"(irb)", "line"=>9, "generation"=>21, "method"=>"to_sym", "flags"=>{"wb_protected"=>true, "old"=>true, "marked"=>true}}

leak_sources
 => {#<ObjspaceHelpers::TrackedObject:0x007f96c9770350 @address=140285597413800, @referenced_by=[]>=>[#<ObjspaceHelpers::TrackedObject:0x007f96c9770328 @address=140285595807360, @referenced_by=[#<ObjspaceHelpers::TrackedObject:0x007f96c9770350 @address=140285597413800, @referenced_by=[]>]>]}

leak_sources.keys.first.info
 => {"type"=>"CLASS", "class"=>140285597413760, "name"=>"SampleClass", "references"=>[140285597413800, 140285597414440, 140285597415800, 140285595807360, 140285594220400], "memsize"=>672, "flags"=>{"wb_protected"=>true, "old"=>true, "marked"=>true}}

leak_sources.values[0][0].info
 => {"type"=>"STRING", "class"=>140285594211920, "embedded"=>true, "bytesize"=>3, "value"=>"yay", "encoding"=>"UTF-8", "references"=>[], "file"=>"(irb)", "line"=>3, "generation"=>21, "method"=>"initialize", "flags"=>{"wb_protected"=>true, "old"=>true, "marked"=>true}}
```

Authors
-------

- [Lukas Fittl](mailto:lukas@fittl.com)

License
-------

Copyright (c) 2014, Lukas Fittl <lukas@fittl.com><br>
objspace_helpers is licensed under the 2-clause BSD license, see LICENSE file for details.
