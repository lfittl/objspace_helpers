$:.push File.expand_path("../lib", __FILE__)
require 'objspace_helpers/version'

Gem::Specification.new do |s|
  s.name        = 'objspace_helpers'
  s.version     = ObjspaceHelpers::VERSION

  s.summary     = 'Helper library to work with ObjectSpace in MRI Ruby 2.1+'
  s.description = 'Diff the heap to find leaks & get other ObjectSpace info more easily.'
  s.author      = 'Lukas Fittl'
  s.email       = 'lukas@fittl.com'
  s.license     = 'BSD-2-Clause'
  s.homepage    = 'http://github.com/lfittl/objspace_helpers'

  s.extensions = %w[ext/objspace_helpers/extconf.rb]

  s.files = %w[
    LICENSE
    Rakefile
    ext/objspace_helpers/extconf.rb
    ext/objspace_helpers/objspace_helpers.c
    ext/objspace_helpers/objspace_helpers.h
    ext/objspace_helpers/objspace_info.c
    ext/objspace_helpers/objspace_info.h
    ext/objspace_helpers/id2ref.c
    ext/objspace_helpers/id2ref.h
    ext/objspace_helpers/ruby_private/gc.h
    ext/objspace_helpers/ruby_private/internal_defs.h
    ext/objspace_helpers/ruby_private/objspace.h
    lib/objspace_helpers.rb
    lib/objspace_helpers/helpers.rb
    lib/objspace_helpers/leaks.rb
    lib/objspace_helpers/tracked_object.rb
    lib/objspace_helpers/version.rb
  ]

  s.add_development_dependency "rake-compiler", '~> 0'
  s.add_development_dependency 'rspec', '~> 3.0'

  s.add_runtime_dependency "json", '~> 1.8'
end
