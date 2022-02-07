pipeline {
    agent any
    environment {
        DISCORD_WEBHOOK_URL     = credentials('discord-webhook')
    }

    stages {
        stage('Build tests') {
            steps {
                sh "cmake test -B build -DCMAKE_BUILD_TYPE=Debug"
                sh "cmake --build build --config Debug --clean-first"
            }
        }

        stage('Run tests') {
            steps {
                dir("build") {
                    sh "./tests --gtest_output=xml:../test_results.xml"
                }
            }
        }

        stage('Build release') {
            when { branch 'master' }
            steps {
                dir("build") {
                    sh "rm -rf build"
                    sh "cmake . -S ${env.WORKSPACE} -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3""
                    sh "cmake --build build --config Release --clean-first"

                    archiveArtifacts(artifacts: "build/tiny", fingerprint: true, onlyIfSuccessful: true)
                }
            }
        }
    }

    post {
        always {
            xunit(
                thresholds: [ skipped(failureThreshold: '0'), failed(failureThreshold: '0') ],
                tools: [ GoogleTest(pattern: 'test_results.xml') ])
        }
        success {
            discordSend(
                description: "The build Nº${env.BUILD_ID} on branch '${env.GIT_BRANCH}' succeeded.",
                link: env.BUILD_URL,
                result: currentBuild.currentResult,
                title: "Build results for branch '${env.GIT_BRANCH}'",
                webhookURL: env.DISCORD_WEBHOOK_URL)
        }
        failure {
            discordSend(
                description: "The build Nº${env.BUILD_ID} on branch '${env.GIT_BRANCH}' failed.",
                link: env.BUILD_URL,
                result: currentBuild.currentResult,
                title: "Build results for branch '${env.GIT_BRANCH}'",
                webhookURL: env.DISCORD_WEBHOOK_URL)
        }
        cleanup {
            deleteDir()
        }
    }
}
