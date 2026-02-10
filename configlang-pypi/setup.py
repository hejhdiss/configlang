from setuptools import setup, Extension
import os
import shutil


setup(
    name='configlang',
    version='1.0.0',
    description='Embedded configuration and automation language library',
    long_description=open('README.md', 'r', encoding='utf-8').read(),
    long_description_content_type='text/markdown',
    author='hejhdiss',
    

    py_modules=['configlang'],
    
    package_data={
        '': ['configlang.dll'],
    },
    include_package_data=True,
    
    python_requires='>=3.6',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 3',
        'Programming Language :: C',
        'Topic :: Software Development :: Libraries',
        'Operating System :: Microsoft :: Windows',
    ],
    keywords='configuration config language embedded automation',
    project_urls={
        'Source': 'https://github.com/hejhdiss/configlang',
        'Bug Reports': 'https://github.com/hejhdiss/configlang/issues',
    },
)