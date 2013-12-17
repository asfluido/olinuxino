MRuby::Gem::Specification::new('olinuxino') do |spec|
  spec.license='public domain'
  spec.author='Carlo E.Prelz'
 
  # Add compile flags
  ['-g','-O6','-funsigned-char','-fPIC','-ffast-math','-Werror','-Wall','-Wcast-align','-Wno-declaration-after-statement','-Wno-unused-function','-Wno-unused-but-set-variable'].each do |f|
    spec.cc.flags.push(f) 	
  end

  # Add cflags to all
  # spec.mruby.cc.flags << '-g'

  # Add libraries
  # spec.linker.libraries.push('jemalloc')
  spec.linker.libraries.push('pthread')

  # Default build files
  # spec.rbfiles = Dir.glob("#{dir}/mrblib/*.rb")
  # spec.objs = Dir.glob("#{dir}/src/*.{c,cpp,m,asm,S}").map { |f| objfile(f.relative_path_from(dir).pathmap("#{build_dir}/%X")) }
  # spec.test_rbfiles = Dir.glob("#{dir}/test/*.rb")
  # spec.test_objs = Dir.glob("#{dir}/test/*.{c,cpp,m,asm,S}").map { |f| objfile(f.relative_path_from(dir).pathmap("#{build_dir}/%X")) }
  # spec.test_preload = 'test/assert.rb'

  # Values accessible as TEST_ARGS inside test scripts
  # spec.test_args = {'tmp_dir' => Dir::tmpdir}
end

