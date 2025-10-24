project(
    'conways-game-of-life', 
    'c',
    meson_version : '>= 1.3.0',
    version : '0.1',
    default_options : ['warning_level=3', 'c_std=c99', 'werror=true'],
)

sdl3_dep = dependency('sdl3', default_options : ['werror=false'])

executable(
    'cgol',
    'src/cgol.c',
    dependencies : [sdl3_dep],
    install : true,
)
