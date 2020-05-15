pipeline {
  agent {
    dockerfile {
      label 'docker'
      additionalBuildArgs '--build-arg K_COMMIT=$(cat deps/k_release | cut --delimiter="-" --field="2") --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g)'
    }
  }
  options { ansiColor('xterm') }
  stages {
    stage('Init title') {
      when { changeRequest() }
      steps { script { currentBuild.displayName = "PR ${env.CHANGE_ID}: ${env.CHANGE_TITLE}" } }
    }
    stage('Test compilation') {
      when { changeRequest() }
      steps {
        sh '''
          make -j16 libff
          make -j16
        '''
      }
    }
    stage('Deploy') {
      when { branch 'master' }
      steps {
        build job: 'rv-devops/master', propagate: false, wait: false                                  \
            , parameters: [ booleanParam(name: 'UPDATE_DEPS_SUBMODULE', value: true)                  \
                          , string(name: 'PR_REVIEWER', value: 'ehildenb')                            \
                          , string(name: 'UPDATE_DEPS_REPOSITORY', value: 'kframework/evm-semantics') \
                          , string(name: 'UPDATE_DEPS_SUBMODULE_DIR', value: 'deps/plugin')           \
                          ]
      }
    }
  }
}
