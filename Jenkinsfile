pipeline {
  agent {
    dockerfile {
      label 'docker'
      additionalBuildArgs '--build-arg K_COMMIT=$(cat deps/k_release | cut --characters=2-) --build-arg USER_ID=$(id -u) --build-arg GROUP_ID=$(id -g)'
    }
  }
  options { ansiColor('xterm') }
  environment { LONG_REV = """${sh(returnStdout: true, script: 'git rev-parse HEAD').trim()}""" }
  stages {
    stage('Init title') {
      when { changeRequest() }
      steps { script { currentBuild.displayName = "PR ${env.CHANGE_ID}: ${env.CHANGE_TITLE}" } }
    }
    stage('Test compilation') {
      when { changeRequest() }
      steps {
        sh '''
          make -j16 CXX=clang++-8 libcryptopp libff
          make -j16 CXX=clang++-8
        '''
      }
    }
    stage('Deploy') {
      when { branch 'master' }
      steps {
        build job: 'rv-devops/master', propagate: false, wait: false                                                        \
            , parameters: [ booleanParam ( name: 'UPDATE_DEPS'         , value: true                                      ) \
                          , string       ( name: 'UPDATE_DEPS_REPO'    , value: 'runtimeverification/blockchain-k-plugin' ) \
                          , string       ( name: 'UPDATE_DEPS_VERSION' , value: "${env.LONG_REV}")                          \
                          ]
      }
    }
  }
}
