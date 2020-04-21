openblas_dep = dependency('openblas')

# FIXME: the openblas pkg-config should add himself
# -lm -lpthread
cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)


cblas_tests = [
    {'name': 'test-cblas-dgemm-1', 'sources': 'test-cblas-dgemm-1.c'},
    {'name': 'test-cblas-dgemm-1b', 'sources': 'test-cblas-dgemm-1b.c'},
    {'name': 'test-cblas-dgemm-2', 'sources': 'test-cblas-dgemm-2.c'},
]

foreach test : cblas_tests
    test_exec = executable(test['name'], test['sources'],
        dependencies: [openblas_dep, m_dep, threads_dep],
        install: false,
    )
    test('Test: ' + test['name'], test_exec)
endforeach
