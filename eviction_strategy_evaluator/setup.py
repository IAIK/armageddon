from codecs import open as codecs_open
from setuptools import setup, find_packages


# Get the long description from the relevant file
with codecs_open('README.md', encoding='utf-8') as f:
    long_description = f.read()


setup(name='eviction_strategy_evaluator',
      version='0.0.1',
      description=u"Eviction strategy evaluator for libflush",
      long_description=long_description,
      classifiers=[],
      keywords='',
      author=u"Moritz Lipp",
      author_email='mail@mlq.me',
      url='https://github.com/iaik/armageddon',
      license='zlib',
      packages=find_packages(exclude=['ez_setup', 'examples', 'tests']),
      package_data={"eviction_strategy_evaluator": [
          "templates/strategy.jinja2",
          "source/Makefile",
          "source/main.c"
         ]},
      include_package_data=True,
      zip_safe=False,
      install_requires=[
          'click',
          'PyYAML',
          'pandas'
      ],
      extras_require={
          'test': ['pytest'],
      },
      entry_points="""
      [console_scripts]
      eviction_strategy_evaluator=eviction_strategy_evaluator.main:main
      """
      )
