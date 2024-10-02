project('jack_tarea4','cpp',
        default_options : ['cpp_std=c++20'],  
        version : '0.0.2')

# Find PipeWire dependency
jack_dep = dependency('jack')
sndfile_dep = dependency('sndfile')
boost_dep = dependency('boost', modules : ['program_options'])
fftw_dep = dependency('fftw3f')

all_deps = [jack_dep,sndfile_dep,boost_dep,fftw_dep]

sources = files('main.cpp', 'jack_client.cpp','dsp_client.cpp',
                'waitkey.cpp','sndfile_thread.cpp', #'freq_filter.cpp', 
                'freq_filter_adder.cpp')

executable('tarea4',sources,dependencies:all_deps)
